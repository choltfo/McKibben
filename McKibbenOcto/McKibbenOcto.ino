// McKibben pneumatic muscle actuated tail controller.
// Charles Holtforster
// 2019

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
const int* avalves[2] = {(void*)xvalves, (void*)yvalves};

// Indicators on the PCB
const int LEDs[2] = {10, 11};

// Buttons on the PCB.
const int button[2] = {12, 13};

// Pins connected to the darlington array and 5V JST open-collector outputs.
const int haptics[4] = {3, 5, 6, 9};

// X and Y ADCs. The 0th should be X
const int ADCS[2] = {A4, A5};

// Contoller timing
const int F = 20; // PWM frequency
const int T = 1000/F; // PWM period
const int T_UP = 1; // ms, time between update cycles. Represents PWM resolution.

// Trim values. Set at boot.
int trims[2] = {1024/2, 1024/2};

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 8; ++i) {
    pinMode(valve[i], OUTPUT);
    digitalWrite(valve[i], LOW);
  }
  for (int i = 0; i < 2; ++i) {
    pinMode(LEDs[i], OUTPUT);
    digitalWrite(LEDs[i], LOW);
  }
  for (int i = 0; i < 2; ++i) {
    trims[i] = analogRead(ADCS[i]);
  }
}

const int DB = 50; // Deadband from zero, radial.
const int CO = 450;// input for maximum ouptput.

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

  // Run the X and Y axis
  for (int axis = 0; axis < 2; ++axis) {
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

  delay(T_UP); // Run
}
