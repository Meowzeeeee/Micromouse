#ifndef TOUCH_SENSOR_H
#define TOUCH_SENSOR_H

#include "sensor.h"

class TouchSensor : public Sensor {
public:
    bool detect(const Maze &maze, const Robot &robot) override;
};

#endif // TOUCH_SENSOR_H
