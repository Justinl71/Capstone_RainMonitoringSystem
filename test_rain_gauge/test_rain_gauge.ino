#define RAIN_GAUGE 3
static unsigned long last_interrupt_time = 0;
unsigned long interrupt_time = millis();
static float daily_rain_measurement = 0; // in mm

void setup() {
  // put your setup code here, to run once:
  pinMode(RAIN_GAUGE, INPUT);
  Serial.begin(9600);
  Serial.println("Ready!!!");
  attachInterrupt(digitalPinToInterrupt(RAIN_GAUGE), rain_gauge_interrupt, FALLING);
}
void rain_gauge_interrupt() {

  detachInterrupt(digitalPinToInterrupt(RAIN_GAUGE));

  // For debouncing
  interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 1000)
  {
    // Do interrupt stuff here
    Serial.println("Rain gauge pulsed");
    daily_rain_measurement = daily_rain_measurement + 0.3;
  }
  last_interrupt_time = interrupt_time;

  attachInterrupt(digitalPinToInterrupt(RAIN_GAUGE), rain_gauge_interrupt, FALLING);

}
void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("Current Rain measurement: ");
  Serial.println(daily_rain_measurement,3);
  delay(1000);
}
