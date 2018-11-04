#include <RunningMedian.h>

#define ECHO_PIN D7 // Echo Pin
#define TRIG_PIN D6 // Trigger Pin
#define PIN_LED 2 // Pin for ESP built-in pin
#define MAX_DISTANCE 200  // Maximum distance to be accepted from HC-SR04 calculations (cm)
#define MIN_DISTANCE_CHANGE 15  // Minimum ditance change triggering obstacle detection (cm)
#define MIN_OBSTACLE_DETECTIONS 7  // Minimum of distance change detected to trigger obstacle detection
#define LONG_SAMPLE_SIZE 50  // Number of samples in distance rolling average
#define SHORT_SAMPLE_SIZE 5  // Number of samples in distance rolling average

long duration, raw_distance, previous_distance,current_distance; // Variables used to get and filter distance
int obstacle_detected=0,no_obstacle_detected=0;
int confirmed_passages=0;  // Number of valid passages detected
bool detection_triggered=false;  // Gets triggered when detected distance reduces due to a valid obstacle
RunningMedian distance_long_sample = RunningMedian(LONG_SAMPLE_SIZE);  // Sample of last SAMPLE_SIZE distances - Used to calculate rolling average
RunningMedian distance_short_sample = RunningMedian(SHORT_SAMPLE_SIZE);  // Sample of last SAMPLE_SIZE distances - Used to calculate rolling average

void GetDistance(){
  /* The following TRIG_PIN/ECHO_PIN cycle is used to determine the
  distance of the nearest object by bouncing soundwaves off of it. */
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  raw_distance = duration/58.2;  //Calculate the distance (in cm) based on the speed of sound.
  if (raw_distance<=MAX_DISTANCE && abs(current_distance-previous_distance)<=MIN_DISTANCE_CHANGE){
    current_distance=raw_distance;
    previous_distance=current_distance;
    distance_long_sample.add(current_distance);
    distance_short_sample.add(current_distance);
  }
  delay(10);  //Delay before next reading.
}

 void CheckForObstacles() {
   // Contains rules established to filter live distances and count valid obstacles
   if (distance_long_sample.getAverage()<120){digitalWrite(PIN_LED,LOW);} else {digitalWrite(PIN_LED,HIGH);}
   if (distance_long_sample.getAverage()-distance_short_sample.getAverage()>MIN_DISTANCE_CHANGE){obstacle_detected++;} else {obstacle_detected=0;}
   if (obstacle_detected>=MIN_OBSTACLE_DETECTIONS){detection_triggered=true;}
   if (detection_triggered && distance_long_sample.getAverage()-distance_short_sample.getAverage()<MIN_DISTANCE_CHANGE){no_obstacle_detected++;} else {no_obstacle_detected=0;}
   if (detection_triggered && no_obstacle_detected>=MIN_OBSTACLE_DETECTIONS){
     confirmed_passages++;
     obstacle_detected=0;
     no_obstacle_detected=0;
     detection_triggered=false;
   }
 }
