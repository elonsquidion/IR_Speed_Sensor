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
float mass = 0.035; // kg
float diameter = 3.7;
float radius = diameter / 2; // all in centimeter
int sensorPin = 2;
int count = 0;
unsigned long perimeter = PI * diameter;
bool move = false;
bool constant = false;

void setup() {
  pinMode(r1, OUTPUT);
  pinMode(sensorPin, INPUT_PULLUP);
  // Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(2), Pulse_Event, RISING);
  digitalWrite(r1, HIGH);
  delay(1000);  
}

void loop() {
  // Just line of code to check the relay function. 
  // digitalWrite(r1, LOW);
  // delay(2000);
  // digitalWrite(r1, HIGH);
  // delay(2000);

  stopTime = 0;

  // start time when car starts moving
  // if (RPM > 0 && move == false){
  //   startTime = millis();
  //   move = true;
  // }

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
    // count++;
    if (startTime == 0){    
      startTime = millis();
    } else {
      stopTime = millis();
      elapsedTime = stopTime - startTime;
      // measure the distance
      distance += RPM * PI / 30000 * radius * elapsedTime;  // yeah boi riemann sum
      startTime = millis();
    }
    // delay(100);  // to avoid double measurement for just one event.
  }


  // check the RPM and setup timer  
  // if (constant == false && move){
  //   stopTime = millis();
  //   elapsedTime = stopTime - startTime;  // time needed for car reaches its terminal velocity
  //   if abs(RPM - average <= 10){
  //     constant = true;  // relatively constant
        
  


  // predict the future
  /*
  convert RPM to rad/s, 1 RPM = 2PI/60 rad/s. So, 1 RPM = 1/30 PI rad/s. The result should be multiplying by radius to get linear velocity.
  elapsed time in milisecond so it needs to be converted to second by dividing it by 1000. 1 ms = 1/1000 s.

  for predicted values, we should calculate the area of triangle which has base of elapsed time and height of linear velocity.
  Thus, predicted values = distance + 1/2 * elapsed time * 1/1000 * 1/30 * PI * radius * RPM.

  NEVERMIND ALL OF THAT

  make an asumption that mk = 0.3 and t = v/mkg
  so, 1/2 * t * RPM * PI / 30 radius
  */

  unsigned long vStop = RPM * PI / 30 * radius;
  unsigned long time = vStop / (0.3 * gravity);
  unsigned long predicted = distance + 1/2 * vStop * time;
  if (predicted >= target) {
    digitalWrite(r1, LOW);  // stop the current
  }
  // stopTime = millis();
  // elapsedTime = stopTime - startTime;  // time needed for car reaches its terminal velocity

  // Print everything in Serial Monitor, TX lamp will kedap-kedip
  // Serial.print("One time loop: ");
  // Serial.println(elapsedTime * 1000);
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
  // Serial.print("\tCount: ");sssssss
  // Serial.print(count);
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
