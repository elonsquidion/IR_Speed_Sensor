#define r1 8
#define PI 3.1415926535897932384626433832795
#define gravity 980.67  // cm/s^2

const byte PulsesPerRevolution = 2;
const unsigned long ZeroTimeout = 100000;
const byte numReadings = 2;

volatile unsigned long LastTimeWeMeasured;
volatile unsigned long PeriodBetweenPulses = ZeroTimeout + 1000;
volatile unsigned long PeriodAverage = ZeroTimeout + 1000;
unsigned long FrequencyRaw;
unsigned long FrequencyReal;
unsigned long RPM;
unsigned int PulseCounter = 1;
unsigned long PeriodSum;

unsigned long startTime;
unsigned long stopTime;
unsigned long elapsedTime = 0;
unsigned long LastTimeCycleMeasure = LastTimeWeMeasured;
unsigned long CurrentMicros = micros();
unsigned int AmountOfReadings = 1;
unsigned int ZeroDebouncingExtra;
unsigned long readings[numReadings];
unsigned long readIndex;  
unsigned long total; 
unsigned long average;
unsigned long distance = 0;
unsigned long target = 3000;
float diameter = 3.7;
float radius = diameter / 2; // all in centimeters
int sensorPin = 2;

void setup() {
  pinMode(r1, OUTPUT);
  pinMode(sensorPin, INPUT_PULLUP);
  // Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(2), Pulse_Event, RISING);
  digitalWrite(r1, HIGH);
  delay(1000);  
}

void loop() {
  stopTime = 0;

  // code that I stole from internet to measure RPM and I have no idea how it works
  LastTimeCycleMeasure = LastTimeWeMeasured;
  CurrentMicros = micros();
  if (CurrentMicros < LastTimeCycleMeasure) {
    LastTimeCycleMeasure = CurrentMicros;
  }
  FrequencyRaw = 10000000000 / PeriodAverage;
  if (PeriodBetweenPulses > ZeroTimeout - ZeroDebouncingExtra || CurrentMicros - LastTimeCycleMeasure > ZeroTimeout - ZeroDebouncingExtra) {
    FrequencyRaw = 0;  // Set frequency as 0.
    ZeroDebouncingExtra = 2000;
  } else {
    ZeroDebouncingExtra = 0;
  }
  FrequencyReal = FrequencyRaw / 10000;
  RPM = FrequencyRaw / PulsesPerRevolution * 60;
  RPM = RPM / 10000;
  total = total - readings[readIndex];
  readings[readIndex] = RPM;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  average = total / numReadings;

  // check if something passed IR sensor
  if(digitalRead(sensorPin) == HIGH) {
    if (startTime == 0){    
      startTime = millis();
    } else {
      stopTime = millis();
      elapsedTime = stopTime - startTime;

      // measure the distance
      /*
      1 RPM is 2PI/60 rad/s or PI/30 rad/s.
      elapsedTime is in miliseconds so it needs to be divided by 1000.
      the distance is the area of a square that is (velocity x time) or RPM PI/30 1/1000 t  
      */
      distance += RPM * PI / 30000 * radius * elapsedTime;  // yeah boi riemann sum
      startTime = millis();
    }
  }

  // predict the future
  /*
  make an asumption that mk = 0.3 and t = v/mkg
  so, future distance = 1/2 * t * RPM * PI / 30 radius (area of a triangle)
  */

  unsigned long vStop = RPM * PI / 30 * radius;
  unsigned long time = vStop / (0.3 * gravity);
  unsigned long predicted = distance + 1/2 * vStop * time;
  if (predicted >= target) {
    digitalWrite(r1, LOW);  // stop the current
  }

  // Print everything in Serial Monitor
  // Serial.print("Period: ");
  // Serial.print(PeriodBetweenPulses);
  // Serial.print("\tReadings: ");
  // Serial.print(AmountOfReadings);
  // Serial.print("\tFrequency: ");
  // Serial.print(FrequencyReal);
  // Serial.print("Perimeter: ");
  // Serial.print(perimeter);
  // Serial.print("Stats: ");
  // Serial.print(constant);
  // Serial.print("\tMove: ");
  // Serial.print(move);
  // Serial.print("\tTachometer: ");
  // Serial.print(average);
  // Serial.print("\tRPM: ");
  // Serial.print(RPM);
  // Serial.print("\tDistance: ");
  // Serial.print(distance);
  // Serial.print("\Predicted: ");
  // Serial.println(predicted);
}

// it is also the code that I stole from internet.
void Pulse_Event() {
  PeriodBetweenPulses = micros() - LastTimeWeMeasured;
  LastTimeWeMeasured = micros(); 
  if (PulseCounter >= AmountOfReadings)  {
    PeriodAverage = PeriodSum / AmountOfReadings;
    PulseCounter = 1;
    PeriodSum = PeriodBetweenPulses;

    int RemapedAmountOfReadings = map(PeriodBetweenPulses, 40000, 5000, 1, 10);
    RemapedAmountOfReadings = constrain(RemapedAmountOfReadings, 1, 10);
    AmountOfReadings = RemapedAmountOfReadings;
  } else {
    PulseCounter++;
    PeriodSum = PeriodSum + PeriodBetweenPulses;
  }
}
