
const int led = 13;
const int pwRouter = 4;
const int pwRPi    = 5;
const int inActivity = A0;  // GPIO25
const int inRouter   = A1;  // GPIO24
const int inRPi      = A2;  // GPIO23


const int dev[3] = { led, pwRouter, pwRPi },
          sig[3] = { inActivity, inRouter, inRPi };
boolean state[3] = {false, false, false};

unsigned long lastActivity = 0;


void setup() {                
//  Serial.begin(115200);

  // initialize control pins as an output.
  for (int i=0; i<3; ++i)  pinMode(dev[i], OUTPUT);
  
  // analog pins are input by default after reset
//  pinMode(inRouter, INPUT);
//  pinMode(inRPi, INPUT);
//  pinMode(inActivity, INPUT);

  lastActivity = millis();  // reset is last acitivty
}

//unsigned long lastDebugPrint = 0;

void loop() {
  unsigned long curTime = millis();
/*
  // just debug printout
  if (curTime-lastDebugPrint > 1000) {
    char buffer[100];
    sprintf(buffer, "Activity=%d, Router=%d, RPi=%d", analogRead(inActivity), analogRead(inRouter), analogRead(inRPi));
    Serial.println(buffer);
    lastDebugPrint = curTime;
  }
*/
  // check activity
  if (state[0] != (analogRead(inActivity)>500)) {  // activity signal changed
    lastActivity = curTime;
  }

  // check absence of activity for the long time
  if (curTime - lastActivity > 10 * 60 * 1000L) {  // if more than 10 minutes since last activity - reset
    digitalWrite(pwRPi, HIGH);    // switch power off
    delay(10000);                 // wait 10 seconds
    digitalWrite(pwRPi, LOW);     // switch power back on
    lastActivity = millis();
  }

  // check direct manipulation and adjust states  
  for (int i=0; i<3; ++i) {
    if (state[i] != (analogRead(sig[i])>500)) {
      state[i] = !state[i];
      digitalWrite(dev[i], state[i]);
    }
  }
}
