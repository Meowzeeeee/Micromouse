#ifndef ROTATING_ULTRASONIC_SENSOR_H
#define ROTATING_ULTRASONIC_SENSOR_H

#include "sensor.h"
#include "direction.h"
#include <array>

class Maze;
class Robot;

class RotatingUltrasonicSensor : public Sensor {
public:
    // range – maksymalny zasięg (w liczbie pól)
    explicit RotatingUltrasonicSensor(int range);

    // Metoda aktualizująca pomiary. Powinna być wywoływana raz na cykl symulacji.
    void update(const Maze &maze, const Robot &robot);

    // Zwraca zmierzony dystans dla relatywnego kierunku: Left (0), Front (1), Right (2).
    int getDistance(Direction relativeDir) const;

    // Dla zgodności z interfejsem Sensor
    bool detect(const Maze &maze, const Robot &robot) override { return false; }

private:
    int m_range;
    // Indeksy: 0 - lewo, 1 - przód, 2 - prawo
    std::array<int, 3> m_measurements;
    // Aktualny indeks skanowania, który przy kolejnym update będzie przesunięty
    int m_currentIndex;
};

#endif // ROTATING_ULTRASONIC_SENSOR_H
