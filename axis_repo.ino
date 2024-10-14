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
void doTarget(char* cmd) {
    command.scalar(&target_velocity, cmd);
}

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
    motor.voltage_limit = 12;
    
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

// PID controller with appropriate constructor
PIDController P_haptic_spring(20, 0, 0.1, 50000, 8);
// attractor angle variable
float attract_angle = 0;
// distance between attraction points
float attractor_distance = 10 * _PI / 180.0;

float findAttractor(float current_angle) {
    return round(current_angle / attractor_distance) * attractor_distance;
}

void loop() {
    // main FOC algorithm function
    motor.loopFOC();

    // Motion control function
    motor.move(P_haptic_spring(attract_angle - motor.shaft_angle));

    // calculate the attractor
    attract_angle = findAttractor(motor.shaft_angle);
}
