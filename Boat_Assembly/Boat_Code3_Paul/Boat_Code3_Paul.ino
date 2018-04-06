#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <SoftwareSerial.h>

//SoftwareSerial XBee(2, 3); // RX, TX
//StreamSend::sendObject(Serial, &charVariable, sizeof(charVariable));

//I^2C /Addresses
//#define MSTR_ADDR 0x24      //Master Address
//#define BCN_ADDR 0x16       //Beacon Address
#define CTRL_ADDR 0x11      //Motor Controller
#define GYR_ADDR 0x09       //Gyroscope Address

/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (25)
Adafruit_BNO055 bno = Adafruit_BNO055();

double offset=0.025;


float pos[5] = {0};  //x y Position Array of Boat
#define Xb pos[0]    //Current Boat X-coordinate
#define Yb pos[1]    //Current Boat Y-coordinate
#define theta_x2 pos[2]    //Current Boat Y-coordinate
#define dx pos[3]   //Start Point X-coordinate   //dx=x_ref-x_rel; x_ref=sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1))+offset;//m Displacement between two waypoints (x1,y1) and (x2, y2)
#define theta_set pos[4]   //Start Point Y-coordinate //theta_set Desired Angle determined by waypoints (x1,y1) and (x2, y2)
double x_rel;        //Current Relative Position of Robot //x_rel=sqrt((yR-y1)*(yR-y1)+(xR-x1)*(xR-x1));//m Displacement of the Boat from the last waypoint (x1, y1)

//Left Motor
#define L0 4
#define L1 7
#define LPWM 5  //enable

//Right Motor
#define R0 10
#define R1 11
#define RPWM 6  //enable

#define pi 3.14159265359
double u_max=3.593*0.0254;//m/s @ 45RPM = 1*2*pi*(45rpm)*r_wheel/(60s/1min) Max Drive motor Speed @ 6V

float gain[3]={0.707,1.3836,0.1};
#define k_i_pos gain[0]
#define k_p_pos gain[1]
#define k_theta gain[2]

int v_lim=200;           //Speed Cap (Nonlinearity) Anti-Saturation Speed Cap 140 out of 255

//Prototypes
void StopMoving(int ctr1, int ctr2, int enable);             //designed specifically for locking drive motors
void MoveMotor(int ctr1, int ctr2, int enable, int duty);    //designed specifically for drive motors
void getPos();
void pwm(int crt1,int duty);                                 //designed specifically for linear actuator and broom
int sign(double number);


void setup() {
  Serial1.begin(57600);
  Wire.begin(CTRL_ADDR);
  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial1.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  int sleep = 8;
  delay(1000); 
  //Define Motor Pins as Outputs
  pinMode(L0, OUTPUT);//Direction
  pinMode(L1, OUTPUT);//Direction
  pinMode(LPWM, OUTPUT);//Left PWM
  pinMode(R0, OUTPUT);//Direction
  pinMode(R1, OUTPUT);//Direction
  pinMode(RPWM,OUTPUT);//Right PWM
  pinMode(sleep,OUTPUT);//Global Enable
  digitalWrite(sleep,HIGH);
}


int z = 0;

    
//IMU data initialization
double theta_x;
double theta_y;
double theta_z;
double d_theta; //Angular Deviation from desired orientation (rad) 


unsigned long time = 0;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  
  if (z == 0) {
    time = 0;
    z=1;
    //Output Pin Initialization
    digitalWrite(L0,LOW);
    digitalWrite(L1,LOW);
    digitalWrite(LPWM,LOW);
    digitalWrite(R0,LOW);
    digitalWrite(R1,LOW);
    digitalWrite(RPWM,LOW);
    delay(5000);//5s break
  }
  /////////////////////////////////////////////////////////////////
  //define current time
  time = millis();
  
  
  //////////////////////////////////////////////////////////////////////////////////////////////
  //Read the Xbee
  
  //Read Sensors
  getPos();//Get Position and References from Camera and Central Computer
  //////////////////////////////////////////////////////////////////////////////////////////////
  //Read the IMU
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  /* Display the floating point data */
  theta_x=2*pi-(euler.x())*pi/180;//put a neg. here to negate Body-fixed Z-axis
  theta_y=(euler.y())*pi/180;// Body-fixed x-axis
  theta_z=(euler.z())*pi/180;// Body-fixed y-axis 
  //IMU Acceleration Vector (No need.)
  //imu::Vector<3> acceler = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  //////////////////////////////////////////////////////////////////
  //Feedforward 2 points from a Curve (We are between 2 waypoints.)
  
  //Get differences between reference and current boat state in both orientation and displacement, respectively.
  d_theta = theta_diff(theta_x,theta_set);// Difference between Desired Angle and Current Boat Angle  //dx=x_ref-x_rel;//m is obtained from xbee.
  
  /////////////////////////////////////////////////////////////////////////////////////////////
  //Initialize Steering and Position Control Variables
  //Note that the critical idea is to avoid saturation of the actuator--> unusable actuator.
  double width=1.8125*0.0254;//m (45 - 46) mm distance between the two differential drive propeller axes
  int fL_st;                                                  //Left side steering control duty cycle
  int fR_st;                                                  //Right side steering control duty cycle
  int fL_pos;                                                 //Left side position control duty cycle
  int fR_pos;                                                 //Right side position control duty cycle
  ///////////////////////////////////////////////////////
  
  //Steering Control Laws (P-control)
  //Left Motor Control Law
  //Gains may need to be adjusted if one motor is faster (better) than another.
  fL_st=constrain(-floor(256*k_theta*(d_theta/u_max)),-200,200);
  //Right Motor Control Law
  fR_st=constrain(floor(256*k_theta*(d_theta/u_max)),-200,200);
  
//////////////////////////////////////////////////////////////  
  //Position Control Law (P-control)
  //Left Side Speed Control Law 
  fL_pos=constrain((floor(256*(k_p_pos*(dx)/u_max))),(-v_lim),v_lim);
  fR_pos=fL_pos;
  
  //Control Law Implentation
  MoveMotor(L0,L1,LPWM,(fL_pos+fL_st));//Left Motor
  MoveMotor(R0,R1,RPWM,(fR_pos+fR_st));//Right Motor
  
  //For Straight-Line testing purposes
  //MoveMotor(L0,L1,LPWM,(150+fL_st));//Left Motor 
  //MoveMotor(R0,R1,RPWM,(150+fR_st));//Right Motor

  /////////////////////////////////////////////////////////////////////////////////////////////
  //Debugging
  //Serial.print("Theta_Z: ");
  //Serial.println(theta_x);
  //Serial.print(" UL:   ");
  //Serial.print((fL_pos+fL_st));
  //Serial.print(",,,");
  //Serial.print(" UR: ");
  Serial.println((fR_pos+fR_st));
  //Serial.println("");
  //Serial.print("dx:");
  //Serial.println(dx);  
  //Serial.print("d_theta:");
  //Serial.println(d_theta);
  delay(10);
  /////////////////////////////////////////////////////////////////////////////////////////////  
 
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Motor Functions
void StopMoving(int ctr1, int ctr2, int enable){//Dead Stop
  digitalWrite(ctr1,LOW);
  digitalWrite(ctr2,LOW);
  digitalWrite(enable,HIGH);  
}

void MoveMotor(int ctr1, int ctr2, int enable,int duty){
  
  if (duty>0){//Move forward
    digitalWrite(ctr1,HIGH);
    digitalWrite(ctr2,LOW);
    if (duty>255){
    digitalWrite(enable,HIGH);
    }
    else{
    analogWrite(enable,duty);
    }
  }
  else if(duty==0){//Smooth Stop
    digitalWrite(ctr1,LOW);
    digitalWrite(ctr2,LOW);
    digitalWrite(enable,LOW);
  }
  else if(duty<0){//Move Backward
    digitalWrite(ctr1,LOW);
    digitalWrite(ctr2,HIGH);
    if (duty<-255){
    digitalWrite(enable,HIGH);
    }
    else{
    analogWrite(enable,abs(duty));
    }
  }
  
  
}

void pwm(int ctr1,int duty){
  analogWrite(ctr1,duty);
}


int sign(double number) { return number<0 ? -1 : 1; }


////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////

//Calculate the difference in theta_z
double theta_diff(float thetax,double thetaset){
  double tole=0.05;
  double d_1=(thetaset-thetax);
  double d_2=(thetaset-thetax+2*pi);
  double d_3=(thetaset-thetax-2*pi);
  d_1=abs(d_1);
  d_2=abs(d_2);
  d_3=abs(d_3);
  
  //Minimize the difference in angle for continuity
  double dtheta=min(d_1,d_2);
  dtheta=min(dtheta,d_3);
  dtheta=abs(dtheta);
  //redefinition of d_theta
  double dth=0;
  if((dtheta<(d_1+tole))&&(dtheta>(d_1-tole))){
    dth=(thetaset-thetax);
  }
  else if((dtheta<(d_2+tole))&&(dtheta>(d_2-tole))) {
    dth=(2*pi+thetaset-thetax);
  }
  else if((dtheta<(d_3+tole))&&(dtheta>(d_3-tole))){
    dth=(thetaset-thetax-2*pi);
  }
  else{
    //do nothing
  }
  return(dth);
}


void getPos() {
  //read the XBee
  //https://stackoverflow.com/questions/14199279/reading-data-over-serial-arduino-and-xbee
  //https://arduino.stackexchange.com/questions/203/sending-large-amounts-of-serial-data
  int i=0;
  while(Serial1.available() and i<5){ 
    pos[i++]= Serial1.parseFloat();//get coordinate array x y
  }
  if(i == 5){
    Serial1.print(pos[1]);
  }
}

