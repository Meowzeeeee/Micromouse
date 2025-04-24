#ifndef SENSOR_H
#define SENSOR_H

class Maze;
class Robot;

class Sensor
{
public:
    // Deklaracja konstruktora
    Sensor();
    virtual ~Sensor() {}

    // Metoda czysto wirtualna, którą klasy pochodne muszą zaimplementować
    virtual bool detect(const Maze &maze, const Robot &robot) = 0;
};

#endif // SENSOR_H
