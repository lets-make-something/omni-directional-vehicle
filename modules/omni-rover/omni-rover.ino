#define DEBUG false

const uint16_t MIDDLE_LEVEL = 4800;
const uint16_t DEFAULT_PPM_CENTER = 1500;
const uint16_t DEFAULT_PPM_CENTER_WORKARROUND4 = 1515;

struct PPM {
  const int PIN;
  int cycle;
  int ch_number;
  bool ready;
  bool falled;
  unsigned long delta;
  unsigned long start;
  unsigned long end;
  int work_ch[9];
  int ch[9];
};

static unsigned long workcycle = 0;

// PWM Setting (ESP32)
//
// 50Hz = 20ms
// 2^16 = 65536
//
// == Theory value ==
// Pulse time : 1 ~ 2ms
// Max    = 6553
// Min    = 3276
// Midium = 4369
//
// == Real value ==
// Max    = 5300
// Midium = 4800
// Min    = 4300
// 
// == in +ch1- -ch2+ -ch4+ ==
// Moter1 = ch2 + ch4
// Moter2 = 2/√3 * ch1 - 2 * ch2 + ch4
// Moter3 = - 2/√3 * ch1 - 2 * ch2 + ch4
//
//   -===+1
//
// +       -
// |       |
//  |2    |3
//  -     +
//
// ch1: x +top, -bottom
// ch2: y +right, -left
// ch4:+right rotate, -left rotate
//
// ECS Setting (Moter)
//
// Throttle
// Min    1232
// Center 1488
// Max    1860
// 

PPM p1 = {18, 0, 0, false, false, 0, 0, 0, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0}};

void IRAM_ATTR isr() {
  if (p1.falled == false) {
    p1.start = micros();
  }
  p1.falled = true;
}


const int ledPin0 = A4;
const int ledPin1 = A5;
const int ledPin2 = A10;

void setup() {
  Serial.begin(115200);
  pinMode(p1.PIN, INPUT_PULLUP);
  attachInterrupt(p1.PIN, isr, FALLING);
  Serial.println("PPM Wait...");
  while (!p1.falled) { 
    Serial.print(".");
    delay(1000);
  }


  ledcSetup(0, 50, 16);
  ledcAttachPin(ledPin0, 0);
  ledcSetup(0, 50, 16);
  ledcAttachPin(ledPin1, 1);
  ledcSetup(0, 50, 16);
  ledcAttachPin(ledPin2, 2);
}


void loop() {
  readPPM();
  writeMoter();  
}

void writeMoter() {
  if (p1.cycle == workcycle) { return; }
  workcycle = p1.cycle;
  
  // Omni motion calculate.
  // == in +ch1- -ch2+ -ch4+ ==
  // Moter1 = ch2 + ch4
  // Moter2 = 2/√3 * ch1 - 2 * ch2 + ch4
  // Moter3 = - 2/√3 * ch1 - 2 * ch2 + ch4
  int moter1 = (p1.ch[1] - DEFAULT_PPM_CENTER)  + (p1.ch[4] - DEFAULT_PPM_CENTER_WORKARROUND4);
  int moter2 = (int)((2 * (p1.ch[0] - DEFAULT_PPM_CENTER) / sqrt(3))  - 2 * (p1.ch[1] - DEFAULT_PPM_CENTER) + (p1.ch[4] - DEFAULT_PPM_CENTER_WORKARROUND4));
  int moter3 = (int)((-2 * (p1.ch[0] - DEFAULT_PPM_CENTER) / sqrt(3))  - 2 * (p1.ch[1] - DEFAULT_PPM_CENTER) + (p1.ch[4] - DEFAULT_PPM_CENTER_WORKARROUND4));
  if (DEBUG == true) {
    Serial.printf("Moter: 1:%d 2:%d 3:%d\n", moter1, moter2, moter3);
  }
  ledcWrite(0, MIDDLE_LEVEL + moter1);
  ledcWrite(1, MIDDLE_LEVEL + moter2);
  ledcWrite(2, MIDDLE_LEVEL + moter3);
  
  // respect motion.
  // ledcWrite(0, MIDDLE_LEVEL + p1.ch[0] - DEFAULT_PPM_CENTER);
  // ledcWrite(1, MIDDLE_LEVEL + p1.ch[1] - DEFAULT_PPM_CENTER);
  // ledcWrite(2, MIDDLE_LEVEL + p1.ch[4] - DEFAULT_PPM_CENTER_WORKARROUND4);  
}

void readPPM() {
  
  //待ステータスを有効にする 
  if (p1.ready == false && p1.falled == true) {
    p1.end = p1.start;
    p1.start = 0;
    p1.falled = false;
    p1.ready = true;
  }

  if (p1.ready == false || p1.falled == false) { return; }
  
  p1.delta = micros() - p1.end;

  //待ステータス初期化
  p1.ready = false;
  p1.falled = false;

  // Cycle end
  if (p1.delta > 6000) {
    if (p1.ch_number == 8) {
      if (DEBUG == true) {
        Serial.printf("DEBUG DELTA: %d %d:%d\n", p1.delta, p1.cycle, p1.ch_number);
        for (int i=0; i<9; i++) {
          Serial.printf("%u ", p1.work_ch[i]);
        }
        Serial.println("");
      }
      for(int i=0; i<9; i++) {
        p1.ch[i] = p1.work_ch[i];
      }
    } else {
      if (DEBUG == true)
        Serial.printf("ERROR %d %d:%d\n", p1.delta, p1.cycle, p1.ch_number);
    }
    p1.ch_number = 0;
    p1.cycle++;
    for(int i=0; i<9; i++) {
      p1.work_ch[i] = 0;
    }
    return;
  }

  // Track channel
  if (p1.ch_number < 9) {
    p1.work_ch[p1.ch_number++] = p1.delta;
  } else {
    if (DEBUG == true)
      Serial.print(".");
  }
}
