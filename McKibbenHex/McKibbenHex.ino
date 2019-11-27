const int valve[8] = {8, 7, 4, 2, A3, A2, A1, A0};

const int Xn = 0;
const int Xp = 1;

const int Yn = 2;
const int Yp = 3;

const int Dv = 4;
const int Lv = 5;


const int LEDs[2] = {12, 13};

const int ADCX = A4;
const int ADCY = A5;

const int refDel = 20;
const int states = 4;

const int pulseMultiplier = 100;
const int npulseMultiplier = 300;

int trimx = 1023/2;
int trimy = 1023/2;

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
  trimx = analogRead(ADCX);
  trimy = analogRead(ADCY);
}

const int LINE = 1;
const int DUMP = 0;

int state = 0;

int xp = 0;
int xn = 0;
int yp = 0;
int yn = 0;

void loop() {
  int xcon = analogRead(ADCX) - trimx;
  int ycon = analogRead(ADCY) - trimy;
  
  bool fstate[4] = {0};


  // Unused at the moment
  //long int t = (millis()%(states*refDel))*1024/(states*refDel);

  int db = 100;
  
  // Limit the known state of the system to within a range that we can exceed with stick movement, so that we can force the output with sufficient motion.
  xp = constrain (xp, -300, 300);
  yp = constrain (yp, -300, 300);

  // Limit the exhaust state to something that we can cannot overwhelm, so at the limits of input we stop firing the exhaust.
  xn = constrain (xn, -600, 600);
  yn = constrain (yn, -600, 600);
  

  // Get our negative and positive error terms.
  int xerr = xp - xcon;
  int yerr = yp - ycon;

  int xnerr = xcon - xn;
  int ynerr = ycon - yn;
  
  fstate[Xn] = xerr > db;
  fstate[Xp] = xerr < -db;
  
  fstate[Yn] = yerr > db;
  fstate[Yp] = yerr < -db;
  
  bool dstate[4] = {0}; // Do we dump?
  
  /*dstate[Xn] = !fstate[Xn];
  dstate[Xp] = !fstate[Xp];
  dstate[Yn] = !fstate[Yn];
  dstate[Yp] = !fstate[Yp];*/

  dstate[Xn] = xnerr > db;
  dstate[Xp] = xnerr < -db;
  
  dstate[Yn] = ynerr > db;
  dstate[Yp] = ynerr < -db;

  
  /*for (int i = 0; i < 4; ++i) {
    Serial.print(fstate[i]);
    Serial.print(',');
  }
  for (int i = 0; i < 4; ++i) {
    Serial.print(dstate[i]);
    Serial.print(',');
  }*/
  Serial.print(xp);
  Serial.print(',');
  Serial.print(xn);
  Serial.print(',');
  Serial.print(xerr);
  Serial.print(',');
  Serial.print(xnerr);
  Serial.println("");
  
  // Update controller state ^^

  // Set some intrinsic state variables for the final valve outputs.
  int fcount = 0;
  for (int i = 0; i < 4; ++i) {
    fcount += fstate[i];
  }

  int dcount = 0;
  for (int i = 0; i < 4; ++i) {
    dcount += dstate[i];
  }
  
  // Run the FSM to change the valve bank state.
  if (state == 0) {
    digitalWrite(valve[Dv], dcount > 0);
    digitalWrite(valve[Lv], dcount == 0);
    for (int i = 0; i < 4; ++i) {
      digitalWrite(valve[i], LOW);
    }
  }
  if (state == 1) {
    digitalWrite(valve[Dv], dcount > 0);
    digitalWrite(valve[Lv], dcount == 0);
    for (int i = 0; i < 4; ++i) {
      digitalWrite(valve[i], dstate[i]);
    }
    if (dstate[Xn]) {
      xn += npulseMultiplier;
    }
    if (dstate[Xp]) {
      xn -= npulseMultiplier;
    }
    if (dstate[Yn]) {
      yn += npulseMultiplier;
    }
    if (dstate[Yp]) {
      yn -= npulseMultiplier;
    }
  }
  if (state == 2) {
    digitalWrite(valve[Dv], fcount == 0 && dcount > 0);
    digitalWrite(valve[Lv], fcount > 0 || dcount == 0);
    for (int i = 0; i < 4; ++i) {
      digitalWrite(valve[i], LOW);
    }
  }
  if (state == 3) {
    digitalWrite(valve[Dv], fcount == 0 && dcount > 0);
    digitalWrite(valve[Lv], fcount > 0 || dcount == 0);
    for (int i = 0; i < 4; ++i) {
      digitalWrite(valve[i],fstate[i]);
    }
    if (fstate[Xn]) {
      xp -= pulseMultiplier;
    }
    if (fstate[Xp]) {
      xp += pulseMultiplier;
    }
    if (fstate[Yn]) {
      yp -= pulseMultiplier;
    }
    if (fstate[Yp]) {
      yp += pulseMultiplier;
    }
  }
  
  
  state ++;
  state %= states;
  delay(refDel);
}
