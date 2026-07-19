/*
  Codex Model/Reasoning Controller
  Arduino Nano

  핀 배치:
    로터리 S1(CLK) -> D2
    로터리 S2(DT)  -> D3
    로터리 KEY     -> D4
    버튼 모델변경   -> D5
    버튼 추론변경   -> D6
    OLED SDA       -> A4
    OLED SCL       -> A5

  Serial 프로토콜 (Arduino -> PC), 9600 baud, 줄바꿈으로 구분:
    CMD:MODEL_CMD               -> "/model" + Enter 입력해야 함
    CMD:REASON_CMD               -> "/reasoning" + Enter 입력해야 함
    CMD:SELECT_ENTER:<value>     -> 로터리 클릭 시, 현재 선택된 값 텍스트 입력 + Enter까지 함께 입력해야 함

  동작 모드(mode)는 Arduino 내부 상태로만 관리하고,
  모드 전환 자체는 PC로 "/model" 또는 "/reasoning" 명령만 보낸다.
  로터리 회전은 PC로 아무것도 보내지 않고 OLED 화면만 갱신한다.
  로터리 클릭 시에만 현재 값 + Enter를 CMD:SELECT_ENTER:<value>로 전송한다.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- 핀 정의 ----------
#define PIN_ENC_CLK   2   // CLK
#define PIN_ENC_DT   3   // DT
#define PIN_ENC_SW  4   // SW
#define PIN_BTN_MODEL   5
#define PIN_BTN_REASON  6

// ---------- OLED 설정 ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- 모드 상태 ----------
enum Mode { MODE_NONE, MODE_MODEL, MODE_REASON };
Mode currentMode = MODE_NONE;

// ---------- 리스트 정의 ----------
const char* modelList[] = { "GPT-5.6Luna", "GPT-5.6Terra", "GPT-5.6Sol" };
const int modelListLen = 3;

const char* reasonList[] = { "Light", "Medium", "High", "Xhigh", "Max" };
const int reasonListLen = 5;

int modelIndex = 0;
int reasonIndex = 0;

// ---------- 로터리 엔코더 상태 (인터럽트용) ----------
volatile int encoderDelta = 0;   // 노치 이동이 확정될 때만 +-1 누적됨
volatile int8_t encSeq = 0;      // 노치를 떠난 뒤 지금까지의 이동 흔적(부호)
volatile uint8_t lastBits = 0b11; // 직전 2비트 상태 (0b11 = 정지 노치 위치)

// 노치(정지)를 벗어났다가 다시 노치로 돌아왔을 때의 중간 패턴으로
// 방향을 판별한다. 바운싱이 여러 번 튀어도 노치 복귀 시 한 번만 확정된다.

// ---------- 디바운스용 ----------
unsigned long lastKeyTime = 0;
unsigned long lastBtnModelTime = 0;
unsigned long lastBtnReasonTime = 0;
const unsigned long DEBOUNCE_MS = 200;

// ---------- 함수 선언 ----------
void updateDisplay();
void sendCommand(const String &cmd);
void handleEncoderTurn();

void setup() {
  Serial.begin(9600);

  pinMode(PIN_ENC_CLK, INPUT_PULLUP);
  pinMode(PIN_ENC_DT, INPUT_PULLUP);
  pinMode(PIN_ENC_SW, INPUT_PULLUP);
  pinMode(PIN_BTN_MODEL, INPUT_PULLUP);
  pinMode(PIN_BTN_REASON, INPUT_PULLUP);

  // 인터럽트: S1, S2 둘 다 변화 감지 (노치 복귀 판정에 둘 다 필요)
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), handleEncoderTurn, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_DT), handleEncoderTurn, CHANGE);

  // OLED 초기화 (I2C 주소 0x3C가 SSD1306 128x64 모듈의 일반적인 기본값)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // OLED 초기화 실패 시 시리얼로만 알림 (디스플레이 못 쓰는 상태)
    Serial.println("ERR:OLED_INIT_FAILED");
  }
  display.clearDisplay();
  updateDisplay();
}

void loop() {
  unsigned long now = millis();

  // ---------- 로터리 회전 처리 ----------
  if (encoderDelta != 0) {
    noInterrupts();
    int delta = encoderDelta;
    encoderDelta = 0;
    interrupts();

    if (currentMode == MODE_MODEL) {
      modelIndex = modelIndex + delta;
      if (modelIndex < 0) modelIndex = 0;
      if (modelIndex >= modelListLen) modelIndex = modelListLen - 1;
      updateDisplay();
    } else if (currentMode == MODE_REASON) {
      reasonIndex = reasonIndex + delta;
      if (reasonIndex < 0) reasonIndex = 0;
      if (reasonIndex >= reasonListLen) reasonIndex = reasonListLen - 1;
      updateDisplay();
    }
    // MODE_NONE일 때는 회전해도 아무 반응 없음
  }

  // ---------- 로터리 클릭 (KEY) : 값 입력 + Enter까지 한 번에, 이후 모드 초기화 ----------
  if (digitalRead(PIN_ENC_SW) == LOW && (now - lastKeyTime) > DEBOUNCE_MS) {
    lastKeyTime = now;
    if (currentMode == MODE_MODEL) {
      sendCommand(String("CMD:SELECT_ENTER:") + modelList[modelIndex]);
      currentMode = MODE_NONE;
      updateDisplay();
    } else if (currentMode == MODE_REASON) {
      sendCommand(String("CMD:SELECT_ENTER:") + reasonList[reasonIndex]);
      currentMode = MODE_NONE;
      updateDisplay();
    }
    // MODE_NONE이면 아무 것도 전송하지 않음
  }

  // ---------- 버튼: 모델변경 ----------
  if (digitalRead(PIN_BTN_MODEL) == LOW && (now - lastBtnModelTime) > DEBOUNCE_MS) {
    lastBtnModelTime = now;
    currentMode = MODE_MODEL;
    sendCommand("CMD:MODEL_CMD");
    updateDisplay();
  }

  // ---------- 버튼: 추론변경 ----------
  if (digitalRead(PIN_BTN_REASON) == LOW && (now - lastBtnReasonTime) > DEBOUNCE_MS) {
    lastBtnReasonTime = now;
    currentMode = MODE_REASON;
    sendCommand("CMD:REASON_CMD");
    updateDisplay();
  }
}

// 인터럽트 서비스 루틴: 노치를 벗어났다가 정지 위치(0b11)로
// 돌아오는 순간에만 방향을 확정한다. 중간의 바운싱은 무시된다.
void handleEncoderTurn() {
  uint8_t s1 = digitalRead(PIN_ENC_CLK);
  uint8_t s2 = digitalRead(PIN_ENC_DT);
  uint8_t bits = (s1 << 1) | s2;

  if (bits == lastBits) {
    return;  // 변화 없음 (노이즈로 인한 중복 트리거), 무시
  }

  if (lastBits == 0b11) {
    // 노치를 막 벗어나기 시작함 -> 첫 이동 방향으로 이번 회전의 방향을 기록
    if (bits == 0b01) {
      encSeq = 1;
    } else if (bits == 0b10) {
      encSeq = -1;
    }
  }

  if (bits == 0b11 && encSeq != 0) {
    // 노치로 복귀 완료 -> 이번 한 번의 회전으로 확정, 카운트
    encoderDelta += encSeq;
    encSeq = 0;
  }

  lastBits = bits;
}

void sendCommand(const String &cmd) {
  Serial.println(cmd);
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  switch (currentMode) {
    case MODE_NONE:
      display.println("Mode: NONE");
      display.println("");
      display.println("Press MODEL or");
      display.println("REASONING button");
      break;

    case MODE_MODEL:
      display.println("Mode: MODEL");
      display.println("");
      display.setTextSize(2);
      display.println(modelList[modelIndex]);
      break;

    case MODE_REASON:
      display.println("Mode: REASONING");
      display.println("");
      display.setTextSize(2);
      display.println(reasonList[reasonIndex]);
      break;
  }

  display.display();
}
