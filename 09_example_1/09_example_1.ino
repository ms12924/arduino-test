// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100     // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300     // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficent to convert duration to distance

#define N_MEDIAN 30
#define _EMA_ALPHA 0.5

// global variables
unsigned long last_sampling_time;   // unit: msec
float dist_buffer[N_MEDIAN];
int buffer_index = 0;

float dist_ema=0;

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED,OUTPUT);
  pinMode(PIN_TRIG,OUTPUT);
  pinMode(PIN_ECHO,INPUT);
  digitalWrite(PIN_TRIG, LOW);

  // initialize serial port
  Serial.begin(57600);

  for (int i=0; i<N_MEDIAN; i++)
    dist_buffer[i] = _DIST_MAX;

  last_sampling_time = millis();
}

void loop() {
  float dist_raw, dist_median;
  
  // wait until next sampling time. 
  // millis() returns the number of milliseconds since the program started. 
  // will overflow after 50 days.
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // get a distance reading from the USS
  dist_raw = USS_measure(PIN_TRIG,PIN_ECHO);

  dist_ema = _EMA_ALPHA * dist_raw + (1.0 - _EMA_ALPHA) * dist_ema;

  dist_buffer[buffer_index] = dist_raw;
  buffer_index = (buffer_index + 1) % N_MEDIAN;

  dist_median = getMedian(N_MEDIAN);

  // output the distance to the serial port
  Serial.print("Min:");   Serial.print(_DIST_MIN);
  Serial.print(",raw:"); Serial.print(dist_raw);
  Serial.print(",ema:");  Serial.print(dist_ema);
  Serial.print(",median:");  Serial.print(dist_median);
  Serial.print(",Max:");  Serial.print(_DIST_MAX);
  Serial.println("");

  // do something here
  if ((dist_median < _DIST_MIN) || (dist_median > _DIST_MAX))
    digitalWrite(PIN_LED, 1);       // LED OFF
  else
    digitalWrite(PIN_LED, 0);       // LED ON

  // update last sampling time
  last_sampling_time += INTERVAL;
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm
}

float getMedian(int size){
  float temp[N_MEDIAN];
  for (int i=0; i<size; i++)temp[i] = dist_buffer[i];

  for (int i=0; i<size-1; i++)
    for (int j=i+1; j<size; j++)
      if (temp[i] > temp[j]){
        float t = temp[i]; temp[i]=temp[j]; temp[j]=t;
      }
  if (size % 2 ==1) return temp[size/2];
  else return 0.5 * (temp[size/2-1] + temp[size/2]);
  
}
