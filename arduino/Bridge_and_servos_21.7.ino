/*
  Arduino Yún Bridge example

 This example for the Arduino Yún shows how to use the
 Bridge library to access the digital and analog pins
 on the board through REST calls. It demonstrates how
 you can create your own API when using REST style
 calls through the browser.

 Possible commands created in this shetch:

 * "/arduino/digital/13"     -> digitalRead(13)
 * "/arduino/digital/13/1"   -> digitalWrite(13, HIGH)
 * "/arduino/analog/2/123"   -> analogWrite(2, 123)
 * "/arduino/analog/2"       -> analogRead(2)
 * "/arduino/mode/13/input"  -> pinMode(13, INPUT)
 * "/arduino/mode/13/output" -> pinMode(13, OUTPUT)

 This example code is part of the public domain

 http://arduino.cc/en/Tutorial/Bridge

 */

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <Servo.h> 

// Listen to the default port 5555, the Yún webserver
// will forward there all the HTTP requests you send
YunServer server;

Servo myservo;  // create servo object to control a servo 
Servo myservo2;  // create servo object to control a servo 

//-----------------CONSTS-------------------
//TODO - maybe change these values later so they would be onfigurable from the mobile app?
int FullRotationTime = 3000;  //in milliseconds - need to test and check which value is close enough
int unitTime = 3;          // a lot of trial an error should come to play here, depends on the scaling and the units

float directionAngle = 0;
float previousDirectionAngle = 0;
int rotationTime;
int MovingForwardTime;

//points properties
//test drawing
int pointsArray[100][2] = {0};
int numOfPoints = 0;


void setup() {
  Serial.begin(9600);
  // Bridge startup
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);
  myservo.attach(9);
  myservo2.attach(10);

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  // Get clients coming from server
  YunClient client = server.accept();

  // There is a new client?
          // Serial.println("got a client");
  if (client) {
         //Serial.println("got a client");
        //Serial.println(client.readString());
        String command;
        command = client.readStringUntil('/');
        command.trim();
        Serial.println(command);
        if (command == "points") {       
          Serial.println("points" );
          populatePointArray(client);
          client.stop();
          int pointMinusOne = numOfPoints -1 ;
          for (int i = 0; i < pointMinusOne; i++){
            moveFromTo(pointsArray[i],pointsArray[i+1]);
            Serial.println("in loop. i = " );
            Serial.println(i);
            if (directionAngle > 0){
              Serial.println("rotating right");
              //rotate right(right or left?)
              myservo.write(0);
              myservo2.write(0);
            } else {
              Serial.println("rotating left");
              myservo.write(180);
              myservo2.write(180);
            }
            delay(rotationTime); //let it rotate for however much time we calculated it needs
            Serial.println("moving forward");
            myservo.write(0);
            myservo2.write(180);
            delay(MovingForwardTime);
          }
           Serial.println("stopping");
           myservo.write(92);
           myservo2.write(92);
        }  
        else if (command == "servo"){
          String whichServo = client.readStringUntil('/');
          int servoVal = client.parseInt();
          if (whichServo == "left"){
            myservo.write(servoVal);
          } else if (whichServo == "right"){
            myservo2.write(servoVal);            
          } else if (whichServo == "both"){
            if (servoVal == 1){
            myservo.write(0);
            myservo2.write(180);
            } else if (servoVal == -1) {            
              myservo.write(180);
              myservo2.write(0);
            } else{  //stop
              myservo.write(92);
              myservo2.write(92);              
            }
          }
          client.stop();
          delay(50); 
        } 
        else if (command == "settings"){
          String setting = client.readStringUntil('/'); 
          int settingVal = client.parseInt();
          Serial.println("in settings. Setting name is : " + setting + " and the value is - " + (String) settingVal);
          if (setting == "fullrotationtime"){
            FullRotationTime = settingVal;
          } else if (setting == "unitscale"){
            unitTime = settingVal;
          } else if (setting == "resetdrawing"){
            previousDirectionAngle = 0;
          }
          client.stop();
          delay(50);
        }
        else{
          // Close connection and free resources.
          client.stop();
          delay(50); // Poll every 50ms
      }
  }
}

void populatePointArray(YunClient client){
      int valRead = client.parseInt();
    // -8 signifies the end of the points array
    numOfPoints = 0;
    while (valRead != -8){
      int yValue = client.parseInt();
      pointsArray[numOfPoints][0] = valRead;
      pointsArray[numOfPoints][1] = yValue;
      numOfPoints++;
      valRead = client.parseInt();
    }
    Serial.println("done reading numbers");
    for (int i = 0; i < 50; i++){
      Serial.println("[" + (String) pointsArray[i][0] + "," + (String) pointsArray[i][1] + "]" );
    }
}
void moveFromTo(int pt1[] ,int pt2[])
{
  int deltaX = pt2[0] - pt1[0];
  int deltaY = pt2[1] - pt1[1];
  float distBtwnPts = sqrt(pow(deltaX,2) + pow(deltaY,2));
  Serial.println("distance between points - " +(String) distBtwnPts);
  MovingForwardTime = (int) distBtwnPts * unitTime;
  float tempAngle = abs(deltaX/distBtwnPts);
  tempAngle = asin(tempAngle);
  Serial.println("Absolute angle - " +(String) tempAngle);
  if (deltaX >= 0 && deltaY >= 0 ){
      Serial.println("in case 1");
    //shouldn't do anything here - TODO change the ifs order and remove it alltogether
    tempAngle = tempAngle;
  } else if (deltaX > 0 && deltaY <= 0 ){
          Serial.println("in case 2");
      tempAngle = 3.14 - tempAngle;
  } else if (deltaX < 0 && deltaY >= 0 ){
          Serial.println("in case 3");
      tempAngle = -tempAngle;
  } else{
          Serial.println("in case 4");
      tempAngle = -3.14 + tempAngle;
  }
  //TODO - make sure the next line is working!! he we change the absolut angle relating to the absolute coordinates 
  //to also take into account the previous rotation
  directionAngle = tempAngle - previousDirectionAngle;
    //we should never roatate more than 180 degrees
  if (directionAngle > 3.14){
     Serial.println("rotating angle more than 180 degress - correcting");
    directionAngle = directionAngle - 6.28;
  } else if (directionAngle < -3.14){
      Serial.println("rotating angle more than 180 degress - correcting");
      directionAngle = 6.28 + directionAngle;
  }
  previousDirectionAngle = tempAngle;
  Serial.println("angle in radians");
  Serial.println(directionAngle);
  radianToRotation(directionAngle);
}

void radianToRotation(float radianAngle){
  rotationTime = radianAngle * FullRotationTime / 6.283;   //6.283 ~ 2 * pi - so a full circle in radians
  if (rotationTime < 0 ){
    rotationTime = rotationTime * -1;
  }
  Serial.println("Rotation time - ");
  Serial.println(rotationTime);
}
