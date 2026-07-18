"""
Codex Model/Reasoning Controller - PC측 스크립트

역할:
    Arduino Nano가 USB Serial로 보내는 명령을 받아서,
    현재 포커스된 창(VS Code Codex 패널이어야 함)에
    클립보드 붙여넣기(Ctrl+V) 방식으로 텍스트를 입력하고 Enter를 누른다.
    작업표시줄 트레이 아이콘으로 실행/연결 상태를 표시한다.

사전 준비:
    pip install pyserial pyautogui pyperclip pystray pillow

사용 전 확인할 것:
    - Arduino를 USB로 연결한 뒤 아래 SERIAL_PORT 값을
      실제 COM 포트 번호로 바꿔야 한다. (Windows 장치관리자 -> 포트(COM & LPT))
    - 버튼을 누르기 전에 사용자가 직접 VS Code Codex 패널을 클릭해서
      포커스를 맞춰줘야 한다. (자동 포커스 기능 없음)

Serial 프로토콜 (Arduino -> PC), 한 줄씩 수신:
    CMD:MODEL_CMD               -> "/model" 붙여넣기 + Enter
    CMD:REASON_CMD               -> "/reasoning" 붙여넣기 + Enter
    CMD:SELECT_ENTER:<value>     -> <value> 텍스트 붙여넣기 + Enter까지 함께 입력

트레이 아이콘 색상:
    회색  -> 포트 연결 시도 중 / 연결 안 됨
    초록  -> 정상 연결되어 명령 대기 중
    빨강  -> 포트 연결 끊김 또는 오류
"""

import time
import threading
import serial
import pyautogui
import pyperclip
import pystray
from PIL import Image, ImageDraw

# ---------- 설정값 ----------
SERIAL_PORT = "COM9"   # 실제 포트 번호로 반드시 변경할 것
BAUD_RATE = 9600
KEY_DELAY = 0.01        # 키 입력 사이 짧은 지연 (안정성용)

# ---------- 전역 상태 ----------
running = True          # False가 되면 백그라운드 스레드와 트레이 아이콘 모두 종료
tray_icon = None        # pystray Icon 객체 (상태 업데이트용)


def make_icon_image(color: str) -> Image.Image:
    """지정한 색의 단색 원형 아이콘 이미지를 생성한다."""
    size = 64
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.ellipse((4, 4, size - 4, size - 4), fill=color)
    return img


def paste_text(text: str):
    """클립보드에 text를 넣고 Ctrl+V로 붙여넣는다."""
    pyperclip.copy(text)
    time.sleep(KEY_DELAY)
    pyautogui.hotkey("ctrl", "v")
    time.sleep(KEY_DELAY)


def clear_input():
    """현재 입력창의 기존 텍스트를 전체 선택 후 삭제하여 비운다."""
    pyautogui.hotkey("ctrl", "a")
    time.sleep(KEY_DELAY)
    pyautogui.press("delete")
    time.sleep(KEY_DELAY)


def press_enter():
    pyautogui.press("enter")
    time.sleep(KEY_DELAY)


def handle_command(line: str):
    """Arduino가 보낸 한 줄 명령을 해석하고 실행한다."""
    line = line.strip()
    if not line:
        return

    print(f"[수신] {line}")

    if line == "CMD:MODEL_CMD":
        clear_input()
        paste_text("/model")
        press_enter()

    elif line == "CMD:REASON_CMD":
        clear_input()
        paste_text("/reasoning")
        press_enter()

    elif line.startswith("CMD:SELECT_ENTER:"):
        value = line[len("CMD:SELECT_ENTER:"):]
        paste_text(value)
        press_enter()
        clear_input()

    elif line.startswith("ERR:"):
        print(f"[Arduino 오류] {line}")

    else:
        print(f"[알 수 없는 명령, 무시함] {line}")


def serial_worker():
    """Serial 포트를 열고 명령을 계속 수신 처리하는 백그라운드 워커.
    별도 스레드에서 실행되며, 연결 상태에 따라 트레이 아이콘을 갱신한다."""
    global running

    print(f"{SERIAL_PORT} 포트 연결 시도 중...")
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    except serial.SerialException as e:
        print(f"포트 연결 실패: {e}")
        print("COM 포트 번호가 맞는지, 다른 프로그램(Arduino IDE Serial Monitor 등)이")
        print("포트를 이미 점유하고 있지 않은지 확인할 것.")
        set_tray_status("연결 실패", "red")
        return

    # 아두이노 리셋 후 안정화 대기 (보드 연결 시 자동 리셋되는 경우가 많음)
    time.sleep(2)
    print("연결 완료. 명령 대기 중...")
    set_tray_status("연결됨 - 대기 중", "green")

    try:
        while running:
            if ser.in_waiting > 0:
                raw_line = ser.readline()
                try:
                    line = raw_line.decode("utf-8", errors="ignore")
                except UnicodeDecodeError:
                    continue
                handle_command(line)
            else:
                time.sleep(0.01)  # 대기 중 CPU 점유율을 낮추기 위한 짧은 휴식
    except Exception as e:
        print(f"[오류] Serial 처리 중 예외 발생: {e}")
        set_tray_status("오류 발생", "red")
    finally:
        ser.close()


def set_tray_status(text: str, color: str):
    """트레이 아이콘의 이미지와 툴팁 텍스트를 갱신한다."""
    if tray_icon is not None:
        tray_icon.icon = make_icon_image(color)
        tray_icon.title = f"Codex Controller - {text}"


def on_quit(icon, item):
    """트레이 메뉴에서 '종료' 선택 시 호출된다."""
    global running
    running = False
    icon.stop()


def main():
    global tray_icon

    # Serial 통신은 별도 스레드에서 백그라운드로 계속 실행
    worker_thread = threading.Thread(target=serial_worker, daemon=True)
    worker_thread.start()

    # 트레이 아이콘 메뉴 구성 (우클릭 시 '종료' 표시)
    menu = pystray.Menu(pystray.MenuItem("종료", on_quit))
    tray_icon = pystray.Icon(
        "codex_controller",
        make_icon_image("gray"),
        "Codex Controller - 연결 시도 중",
        menu,
    )

    # 트레이 아이콘 실행 (메인 스레드를 여기서 점유, 트레이 우클릭 '종료'로 빠져나옴)
    tray_icon.run()


if __name__ == "__main__":
    main()