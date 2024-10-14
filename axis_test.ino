// Haptic texture "Rugós Érzet" 
// https://community.simplefoc.com/t/haptic-textures/301/3

#include "Arduino.h"
#include "SPI.h"
#include "SimpleFOC.h"
#include "SimpleFOCDrivers.h"
#include "encoders/as5048a/MagneticSensorAS5048A.h"

#define SPI_2_MISO  12
#define SPI_2_MOSI  15
#define SPI_2_SCK   14
#define SENSOR_CS  13 // some digital pin that you're using as the nCS pin

MagneticSensorAS5048A sensor(SENSOR_CS, true);
MbedSPI spi2(SPI_2_MISO, SPI_2_MOSI, SPI_2_SCK);

// BLDC motor & driver instance
BLDCMotor motor = BLDCMotor(11);
BLDCDriver3PWM driver = BLDCDriver3PWM(9, 8, 7, 6);

// velocity set point variable
float target_velocity = 0;
// instantiate the commander
Commander command = Commander(Serial);
void doTarget(char* cmd) { command.scalar(&target_velocity, cmd); }

void setup() {
  spi2.begin();
  // initialise magnetic sensor hardware
  sensor.init(&spi2);
  // link the motor to the sensor
  motor.linkSensor(&sensor);
  // pwm frequency to be used [Hz]
  driver.pwm_frequency = 200000;
  // power supply voltage
  driver.voltage_power_supply = 12;
  driver.init();
  motor.linkDriver(&driver);

  // set motion control loop to be used
  motor.controller = MotionControlType::torque;

  // default voltage_power_supply
  motor.voltage_limit = 8; // Reduced voltage to avoid over-excitation
  
  // use monitoring with serial
  Serial.begin(115200);
  // comment out if not needed
  motor.useMonitoring(Serial);

  // initialize motor
  motor.init();
  // align sensor and start FOC
  motor.initFOC();

  Serial.println(F("Motor ready."));
  
  _delay(1000);
}

// haptic spring effect controller - only Proportional
// The further the handle moves from the center, the greater the opposing force
PIDController P_haptic_spring{.P=20, .I=0, .D=0.1, .output_ramp=50000, .limit=8};

// Spring center angle
float spring_center_angle = 0;

float calculateSpringForce(float current_angle) {
  // The more you move away from the center, the greater the force
  float stiffness = 5.0;  // Spring stiffness
  float max_angle = 180.0 * _PI / 180.0;  // Maximum angle for full spring force (one full revolution)
  if (abs(current_angle - spring_center_angle) < 5.0 * _PI / 180.0) {
    return 0;  // No force within 5 degrees of the center
  }
  float normalized_angle = constrain(current_angle - spring_center_angle, -max_angle, max_angle) / max_angle;
  return -stiffness * normalized_angle * max_angle;
}

void loop() {
  // main FOC algorithm function
  motor.loopFOC();

  // calculate spring force
  float spring_force = calculateSpringForce(motor.shaft_angle);

  // Motion control function with spring-like response
  motor.move(P_haptic_spring(spring_force));
} 
