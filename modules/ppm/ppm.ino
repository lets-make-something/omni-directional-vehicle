#define DEBUG false

struct PPM {
  const int PIN;
  int cycle;
  int ch_number;
  bool ready;
  bool falled;
  unsigned long delta;
  unsigned long start;
  unsigned long end;
  unsigned long ch0;
  unsigned long ch1;
  unsigned long ch2;
  unsigned long ch3;
  unsigned long ch4;
  unsigned long ch5;
  unsigned long ch6;
  unsigned long ch7;
  unsigned long ch8;
};

PPM p1 = {18, 0, 0, false, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void IRAM_ATTR isr() {
  if (p1.falled == false) {
    p1.start = micros();
  }
  p1.falled = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(p1.PIN, INPUT_PULLUP);
  attachInterrupt(p1.PIN, isr, FALLING);
  Serial.println("PPM Wait...");
  while (!p1.falled) { 
    Serial.print(".");
    delay(1000);
  }
}

void loop() {
  ppm_reader();  
}

void ppm_reader() {
  
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

  // cycle end
  if (p1.delta > 6000) {
    if (p1.ch_number == 8) {
      if (DEBUG == true) 
        Serial.printf("DEBUG %d %d:%d\n", p1.delta, p1.cycle, p1.ch_number);
      Serial.printf("%u %u %u %u %u %u %u %u\n", p1.ch0, p1.ch1, p1.ch2, p1.ch3, p1.ch4, p1.ch5, p1.ch6, p1.ch7, p1.ch8);
    } else {
      if (DEBUG == true)
        Serial.printf("ERROR %d %d:%d\n", p1.delta, p1.cycle, p1.ch_number);
    }
    p1.ch_number = 0;
    p1.cycle++;
    p1.ch0 = 0;
    p1.ch1 = 0;
    p1.ch2 = 0;
    p1.ch3 = 0;
    p1.ch4 = 0;
    p1.ch5 = 0;
    p1.ch6 = 0;
    p1.ch7 = 0;
    p1.ch8 = 0;
    return;
  }
  
  switch (p1.ch_number++) {
    case 0:
      p1.ch0 = p1.delta;
    break;
    case 1:
      p1.ch1 = p1.delta;
    break;
    case 2:
      p1.ch2 = p1.delta;
    break;
    case 3:
      p1.ch3 = p1.delta;
    break;
    case 4:
      p1.ch4 = p1.delta;
    break;
    case 5:
      p1.ch5 = p1.delta;
    break;
    case 6:
      p1.ch6 = p1.delta;
    break;
    case 7:
      p1.ch7 = p1.delta;
    break;
    case 8:
      p1.ch8 = p1.delta;
    break;
    default:
      if (DEBUG == true)
        Serial.print(".");
    break;
  }
}
