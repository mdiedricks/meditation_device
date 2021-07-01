#include <RTClib.h>
#include <avr/pgmspace.h>

#define POWERLED 0
#define REDPIN 8
#define GRNPIN 9
#define BLUPIN 10
#define PWR_BTN 4
#define MODE_BTN 2
#define FUNCTION_BTN 3

RTC_DS3231 rtc;

struct deviceState {
  bool isSleeping;
  bool startSession;
  bool inSession;
  uint8_t mode; //index of modes list
  uint8_t duration; //index of durations list
  uint8_t interval; // index of interval list
  uint8_t led_red;
  uint8_t led_green;
  uint8_t led_blue;
};

struct timeObject {
  uint8_t hourUnit;
  uint8_t minuteUnit;
};

struct sessionSettings {
  uint8_t duration; //20
  timeObject endTime; // single value
  timeObject intervalHourNext;
  timeObject intervalMinNext;
  uint8_t intervalAmount; //3
};

volatile uint8_t modeFlag = 1;
volatile uint8_t functionFlag = 1;
unsigned long lastModePress;
bool lastModeState;
unsigned long lastFunctionPress;
bool lastFunctionState;
unsigned long debounceTime = 250;

float pressLength_millis = 0;
const int shortPress_millis = 100;
const int longPress_millis = 2000;

const char *modes[3] = {"SESSION", "DURATION", "INTERVAL"};
const int durations[5] = {5, 10, 15, 20, 30};
const int intervals[4] = {0, 1, 5, 10};

deviceState defaultState = {true, false, false, 1, 2, 3, 255, 255, 255};
sessionSettings deviceSession;
sessionSettings currentSession;

//--------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
  pinMode(PWR_BTN, INPUT_PULLUP);

  Serial.print("::Device is ");
  Serial.println(modes[defaultState.isSleeping] ? "SLEEPING" : "NOT SLEEPING");

  Serial.print("::Device mode: ");
  Serial.println(modes[defaultState.mode]);

  Serial.println("::Setting up default session settings");
  DateTime setupEnd = rtc.now() + TimeSpan(0, 0, 15, 0);
  DateTime setupFirstInterval = rtc.now() + TimeSpan(0, 0, defaultState.interval, 0);
  deviceSession = {
    defaultState.duration,
    {setupEnd.hour(), setupEnd.minute()},
    {setupFirstInterval.hour(), setupFirstInterval.minute()},
    (defaultState.duration / defaultState.interval) - 2
  };

  Serial.println("::Setup interrupts");
  pinMode(MODE_BTN, INPUT_PULLUP);
  pinMode(FUNCTION_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MODE_BTN), ISR_mode, LOW);
  attachInterrupt(digitalPinToInterrupt(FUNCTION_BTN), ISR_function, LOW);

  Serial.println("::Initializing LED strip");
  pinMode(REDPIN, OUTPUT);
  pinMode(GRNPIN, OUTPUT);
  pinMode(BLUPIN, OUTPUT);

  Serial.println("::Initializing RTC");
  if (! rtc.begin()) {
    Serial.println("::Error. Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("::Error. RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

//--------------------------------------------------------------------------------

void loop() {

  //CHECK 1 - POWER BUTTON LONG PRESS
  while (digitalRead(PWR_BTN) == LOW) {
    delay(100);
    pressLength_millis = pressLength_millis + 100;
  }
  if (pressLength_millis >= longPress_millis && !defaultState.inSession) {
    // power off
    pressLength_millis = 0;
    powerDown();
  }
  else if (pressLength_millis >= longPress_millis && defaultState.inSession) {
    // end session
    pressLength_millis = 0;
    sessionStop();
  } else {
    pressLength_millis = 0;
  }

  // CHECK 2 - SESSION START
  if (defaultState.startSession) {
    sessionStart();
  }

  //** CHECK 3 - IN SESSION
  if (defaultState.inSession) {
    DateTime curTime = rtc.now();
    // check if session is finished
    if (curTime.minute() == currentSession.endTime.minuteUnit && curTime.hour() == currentSession.endTime.minuteUnit) {
      sessionStop();
    }
    //check if next interval
    if (true) {

    }
  }

  // CHECK 4 - BUTTON STATE
  if (!defaultState.inSession) {

    if ( (millis() - lastModePress) > debounceTime && modeFlag == 0) {
      modeFlag = 1;
      modeCycle();
    }
    if ( (millis() - lastFunctionPress) > debounceTime && functionFlag == 0) {
      functionFlag = 1;
      modeAction();
    }
  }
}

//--------------------------------------------------------------------------------

// INTERRUPTS =============================================
void ISR_mode() {
  if (modeFlag == 1) {
    lastModePress = millis();
  }
  modeFlag = 0;
}
void ISR_function() {
  if (functionFlag == 1) {
    lastFunctionPress = millis();
  }
  functionFlag = 0;
}

// SETTING FUNCTIONS =============================================
void modeCycle() {
  if (defaultState.mode == 2) {
    defaultState.mode = 0;
  } else {
    defaultState.mode ++;
  }
  modeFlag = 1;
  Serial.print("Current mode: ");
  Serial.println(modes[defaultState.mode]);
}

void modeAction() {
  switch (defaultState.mode) {
    case 0:
      defaultState.startSession = true;
      break;
    case 1:
      durationCycle();
      break;
    case 2:
      intervalCycle();
      break;
  }
  functionFlag = 1;
}

void durationCycle() {
  if (defaultState.duration == 4) {
    defaultState.duration = 0;
  } else {
    defaultState.duration ++;
  }
  Serial.print("...New duration: ");
  Serial.println(durations[defaultState.duration]);
}

void intervalCycle() {
  if (defaultState.interval == 3) {
    defaultState.interval = 0;
  } else {
    defaultState.interval ++;
  }
  Serial.print("...New interval: ");
  Serial.println(intervals[defaultState.interval]);
}

// SESSION CONTROLS =============================================
void sessionStart() {
  Serial.println("Session started");
  toggleLights(204, 153, 255, true);

  defaultState.startSession = false;
  defaultState.inSession = true;

  DateTime sessionEnd = rtc.now() + TimeSpan(0, 0, defaultState.duration, 0);
  DateTime firstInterval;
  if (defaultState.interval > 0) {
    firstInterval = getNextInterval();
  }
  currentSession = {
    defaultState.duration,
    {sessionEnd.hour(), sessionEnd.minute()},
    firstInterval.hour(),
    firstInterval.minute(),
    (defaultState.duration / defaultState.interval) - 2
  };

  doubleChime();
};

DateTime getNextInterval() {
  DateTime returnInterval = rtc.now() + TimeSpan(0, 0, defaultState.interval, 0);
  return returnInterval;
}

void sessionStop() {
  Serial.println("Session ended");
  toggleLights(255, 255, 255, true);

  defaultState.inSession == false;

  doubleChime();
};

void powerDown() {
  Serial.println("Powering down...");
  toggleLights(0, 0, 0, false);
};

// EFFECTS CONTROLS
void toggleLights(uint8_t R, uint8_t G, uint8_t B , bool power) {
  //white 255 255 255
  //sessionPurple 204 153 255
  if (!power) {
    analogWrite(REDPIN, 0);
    analogWrite(GRNPIN, 0);
    analogWrite(BLUPIN, 0);
  } else {
    analogWrite(REDPIN, R);
    analogWrite(GRNPIN, G);
    analogWrite(BLUPIN, B);
  }
};

void oneChime() {
  Serial.println("Dinggg......");
};

void doubleChime() {
  Serial.println("Dinggg...Dinggg...");
};
