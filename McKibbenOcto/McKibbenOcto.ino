// McKibben pneumatic muscle actuated tail controller.
// Charles Holtforster
// 2019

const int axes = 2;

// Valve pin assignments.
// Map the Arduino pin numbers for each valve to an array index.
const int valve[8] = {8, 7, 4, 2, A3, A2, A1, A0};

// Lists of valve indexes corresponding to X and Y actuator pairs.
const int xvalves[4] = {0, 1, 2, 3};
const int yvalves[4] = {4, 5, 6, 7};

// Lookup keys for the _valves arrays.
const int fillP = 0;
const int dumpP = 1;
const int fillN = 2;
const int dumpN = 3;

// An indexable list to both _valves arrays
// valves[avalves[AXIS][VALVE_TYPE] is the pin number.
const int* avalves[axes] = {(void*)xvalves, (void*)yvalves};

// Indicators on the PCB
const int nLEDs = 2;
const int LEDs[nLEDs] = {12, 13};

// Buttons on the PCB.
const int nButtons = 2;
const int button[nButtons] = {11, 10};

const int buttonDump = 0;
const int buttonTrim = 1;

// Pins connected to the darlington array and 5V JST open-collector outputs.
const int nHaptics = 4;
const int haptics[nHaptics] = {3, 5, 6, 9};

// X and Y ADCs. The 0th should be X
const int ADCS[2] = {A4, A5};

// Contoller timing
const int F = 20; // PWM frequency
const int T = 1000/F; // PWM period
const int T_UP = 1; // ms, time between update cycles. Represents PWM resolution.

// Control parameters
const int DB = 50; // Deadband from zero. M(0±DB) = 0
const int CO = 450;// input for maximum ouptput. M([CO,1024]) = 100, M([0,512-CO]) = -100;

// Trim values. Set at boot.
// If the stick is not at the center location at boot, well, we're going to have a problem.
// Furthermore, 
int trims[axes] = {1024/2, 1024/2};

// Read the inverted, pulled-up buttons.
int readButton(int i) {
  return !digitalRead(button[i]);
}

// Set one of the indicator LEDs.
void setLED(int i, int state) {
  digitalWrite(LEDs[i], state);
}

void setup() {
  Serial.begin(115200);

  // Configure all of the pins
  for (int i = 0; i < 8; ++i) {
    pinMode(valve[i], OUTPUT);
    digitalWrite(valve[i], LOW);
  }
  for (int i = 0; i < 4; ++i) {
    pinMode(haptics[i], OUTPUT);
  }
  for (int i = 0; i < 2; ++i) {
    pinMode(LEDs[i], OUTPUT);
    digitalWrite(LEDs[i], LOW);
  }
  for (int i = 0; i < 2; ++i) {
    pinMode(button[i], INPUT);
    digitalWrite(button[i], HIGH); // pull-up
  }
  
  for (int i = 0; i < axes; ++i) {
    trims[i] = analogRead(ADCS[i]);
  }
}

int mapCon(int con) {
  int absc = abs(con);
  if (absc < DB) return 0;
  if (absc > CO) return con > 0 ? 100 : -100;
  if (con > 0) return (con-DB)*100/(CO-DB);
  if (con < 0) return (con+DB)*100/(CO-DB);
  Serial.println("Input error!");
  return 0;
}

void loop() {
  long int t = (millis()%50)*2; // Generate PWM sawtooth wave time.

  int dump = readButton(buttonDump); // There's also another one, but I don't know what it does...
  int retrim = readButton(buttonTrim); // We might need to retrim after a potential brownout.

  if(retrim){
    for (int i = 0; i < axes; ++i) {
      trims[i] = analogRead(ADCS[i]);
    }
  }
  
  if (dump) {
    // Discharge all acuators.
    for (int axis = 0; axis < axes; ++axis) {
      int*axisvalves = avalves[axis];
      digitalWrite(valve[axisvalves[fillP]], LOW);
      digitalWrite(valve[axisvalves[dumpP]], HIGH);
      digitalWrite(valve[axisvalves[fillN]], LOW);
      digitalWrite(valve[axisvalves[dumpN]], HIGH);
    }
  } else {
    // Run the X and Y axis according to the controller inputs
    for (int axis = 0; axis < axes; ++axis) {
      int*axisvalves = avalves[axis];
      int con = mapCon(analogRead(ADCS[axis]) - trims[axis]);
  
      Serial.println(con, DEC);
      Serial.println(t, DEC);
      
      if (con > t) {
        Serial.println("POS");
        // Positive direction motion
        digitalWrite(valve[axisvalves[fillP]], HIGH);
        digitalWrite(valve[axisvalves[dumpP]], LOW);
        digitalWrite(valve[axisvalves[fillN]], LOW);
        digitalWrite(valve[axisvalves[dumpN]], HIGH);
      } else if (-con > t) {
        Serial.println("NEG");
        // Negative direction motion
        digitalWrite(valve[axisvalves[fillP]], LOW);
        digitalWrite(valve[axisvalves[dumpP]], HIGH);
        digitalWrite(valve[axisvalves[fillN]], HIGH);
        digitalWrite(valve[axisvalves[dumpN]], LOW);
      } else {
        Serial.println("NIL");
        // Maintain state
        digitalWrite(valve[axisvalves[fillP]], LOW);
        digitalWrite(valve[axisvalves[dumpP]], LOW);
        digitalWrite(valve[axisvalves[fillN]], LOW);
        digitalWrite(valve[axisvalves[dumpN]], LOW);
      }
    }
  }
  
  delay(T_UP); // Run
}
