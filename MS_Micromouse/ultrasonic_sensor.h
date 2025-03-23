#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include "sensor.h"
#include "direction.h"  // Potrzebne do Direction enum

class UltrasonicSensor : public Sensor {
public:
    // Konstruktor z zakresem i kierunkiem patrzenia
    UltrasonicSensor(int range, Direction direction);
    bool detect(const Maze &maze, const Robot &robot) override;

private:
    int m_range;
    Direction m_direction;
};

#endif // ULTRASONIC_SENSOR_H
