#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include "sensor.h"

class UltrasonicSensor : public Sensor {
public:
    // range – maksymalna liczba komórek, na którą czujnik "widzi"
    UltrasonicSensor(int range);
    bool detect(const Maze &maze, const Robot &robot) override;
private:
    int m_range;
};

#endif // ULTRASONIC_SENSOR_H
