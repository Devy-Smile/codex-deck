<p>
  <details>
    <summary>
      README_kr (클릭하여 펼치기)
    </summary>

# codex-deck

> 작은 버튼 하나로 Codex의 모델과 추론 수준을 빠르게 바꾸는 미니 컨트롤러

`codex-deck`은 Arduino Nano, 택트스위치와 로터리 인코더를 이용해 Codex 모델 및 추론 수준 변경을 간편하게 제어하는 Deck 하드웨어입니다. 키보드에서 자주 사용하는 단축키를 물리 버튼으로 구현해, 모델과 추론 수준을 빠르게 변경할 수 있습니다.

<p align="center">
  <img src="https://img.shields.io/badge/Platform-Windows-0078D4?style=flat-square&logo=windows" alt="Windows">
  <img src="https://img.shields.io/badge/Hardware-Arduino%20Nano-00979D?style=flat-square&logo=arduino" alt="Arduino Nano">
  <img src="https://img.shields.io/badge/Language-C%2B%2B-00599C?style=flat-square&logo=cplusplus&logoColor=white" alt="C++">
  <img src="https://img.shields.io/badge/Language-Python-3776AB?style=flat-square&logo=python&logoColor=white" alt="Python">


</p>

## 주요 기능

| 기능 | 동작 |
| --- | --- |
| 모델 변경 | Codex에서 사용할 모델을 빠르게 전환 |
| 추론 수준 변경 | 원하는 추론 수준으로 간편하게 변경 |
| 단축키 입력 | `Ctrl+C`, `Ctrl+V`, `Ctrl+A`, `Enter` 입력 |
| 상태 확인 | SSD1306 OLED에 현재 상태 표시 |

## 사용 부품

- 1x  Arduino Nano
- 2x  4-pin tact_sw
- 1x  KY-040
- 1x  SSD1306
- 28x  M-M Dupont cable
- 1x  Breadboard

## 동작 예시

```text
        ┌────────────────────────────────────┐
        │            [codex-deck]            │
        │         ┌──────┐   ┌─────┐         │
        │         │ OLED │   │KY040│         │
        │         └──────┘   └─────┘         │
        │      [모델 버튼]  [추론 버튼]       │
        └─────────────────┬──────────────────┘
                          │
                          │ USB Serial Port
                          ▼
                    Codex 입력 제어
```

## 시작

1. "codex_deck.ino"를 아두이노에 업로드하세요.
2. ```bash
   cd ..\codex-deck
   ```
3. ```bash
   py -m PyInstaller --onefile --noconsole main.py
   ```

빌드가 완료되면 ..codex-deck\dist에 "main.exe"로 저장됩니다.

```text
dist/
└── main.exe
```

## 실행

1. Arduino Nano와 OLED, 스위치, KY-040을 연결합니다.
2. `dist/main.exe`를 실행합니다.
3. 버튼과 로터리 인코더로 Codex의 모델 및 추론 수준을 변경합니다.
> Arduino 연결 후 빌드된 main.exe를 실행하세요. 시스템 트레이에 실행됩니다. 우클릭으로 종료 할 수 있습니다.
> 
> **운영체제와 Codex 앱의 포커스 상태에 따라 키 입력이 전달되는 창이 달라질 수 있습니다.**

## 프로젝트 구조

```text
codex-deck/
├── main.py
├── ..
└── dist/main.exe    # PyInstaller 빌드 결과
```

## 개발 목적

Codex를 사용하면서 반복적으로 입력하는 단축키를 물리 인터페이스로 분리해, 작업 흐름을 끊지 않고 모델과 추론 수준을 바꾸는 것을 목표로 합니다.
  </details>
</p>

# codex-deck

> A tiny hardware controller for switching Codex's model and reasoning level with a single button press

`codex-deck` is a mini Deck controller built with an Arduino Nano, tact switches, and a rotary encoder to make switching Codex's model and reasoning level effortless. It turns the keyboard shortcuts you'd normally type by hand into physical buttons, so you can change models and reasoning levels without breaking your workflow.

<p align="center">
  <img src="https://img.shields.io/badge/Platform-Windows-0078D4?style=flat-square&logo=windows" alt="Windows">
  <img src="https://img.shields.io/badge/Hardware-Arduino%20Nano-00979D?style=flat-square&logo=arduino" alt="Arduino Nano">
  <img src="https://img.shields.io/badge/Language-C%2B%2B-00599C?style=flat-square&logo=cplusplus&logoColor=white" alt="C++">
  <img src="https://img.shields.io/badge/Language-Python-3776AB?style=flat-square&logo=python&logoColor=white" alt="Python">

</p>

## Features

| Feature | Description |
| --- | --- |
| Model switching | Quickly switch between Codex models |
| Reasoning level control | Easily adjust the reasoning effort level |
| Shortcut input | Sends `Ctrl+C`, `Ctrl+V`, `Ctrl+A`, `Enter` |
| Status display | Shows the current state on an SSD1306 OLED |

## Hardware Used

- 1x  Arduino Nano
- 2x  4-pin tact_sw
- 1x  KY-040
- 1x  SSD1306
- 28x  M-M Dupont cable
- 1x  Breadboard

## How It Works

```text
        ┌────────────────────────────────────┐
        │            [codex-deck]            │
        │         ┌──────┐   ┌─────┐         │
        │         │ OLED │   │KY040│         │
        │         └──────┘   └─────┘         │
        │      [Model btn] [Reasoning btn]   │
        └─────────────────┬──────────────────┘
                          │
                          │ USB Serial Port
                          ▼
                    Codex Input Control
```

## Start
1. Upload "codex_deck.ino" to your arduino.
2. ```bash
   cd ..\codex-deck
   ```
3. ```bash
   py -m PyInstaller --onefile --noconsole main.py
   ```

Once the build finishes, the executable will be in the `dist` folder.

```text
dist/
└── main.exe
```

## Usage

1. Connect the Arduino Nano to the OLED, switches, and KY-040 rotary encoder.
2. Run `dist/main.exe`.
3. Use the buttons and rotary encoder to change Codex's model and reasoning level.

> Run the built `main.exe` after connecting the Arduino. It launches into the system tray — right-click the tray icon to quit.
>
> **Which window receives the key input depends on your OS and which app currently has focus.**

## Project Structure

```text
codex-deck/
├── main.py
├── ..
└── dist/main.exe    # PyInstaller build output
```

## Motivation

The goal of this project is to move the shortcuts you repeatedly type while using Codex onto a physical interface, so you can switch models and reasoning levels without interrupting your flow.
