#include "maze.h"
#include "direction.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>

Maze::Maze(int width, int height)
    : m_width(width), m_height(height)
{
    initCells();
    generate();
}

void Maze::initCells()
{
    m_cells.resize(m_height);
    for (int r = 0; r < m_height; r++) {
        m_cells[r].resize(m_width);
        for (int c = 0; c < m_width; c++) {
            m_cells[r][c].row = r;
            m_cells[r][c].col = c;
            m_cells[r][c].topWall = true;
            m_cells[r][c].rightWall = true;
            m_cells[r][c].bottomWall = true;
            m_cells[r][c].leftWall = true;
            m_cells[r][c].visited = false;
        }
    }
}

void Maze::generate()
{
    std::srand(std::time(nullptr));
    carveMaze(0, 0);
}

void Maze::carveMaze(int row, int col)
{
    m_cells[row][col].visited = true;
    std::vector<std::pair<int,int>> directions = {
        { -1, 0}, { 0, 1}, { 1, 0}, { 0,-1}
    };

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(directions.begin(), directions.end(), g);

    for (auto &dir : directions)
    {
        int newRow = row + dir.first;
        int newCol = col + dir.second;
        if (newRow >= 0 && newRow < m_height && newCol >= 0 && newCol < m_width)
        {
            if (!m_cells[newRow][newCol].visited)
            {
                if (dir.first == -1 && dir.second == 0) {
                    m_cells[row][col].topWall = false;
                    m_cells[newRow][newCol].bottomWall = false;
                }
                else if (dir.first == 1 && dir.second == 0) {
                    m_cells[row][col].bottomWall = false;
                    m_cells[newRow][newCol].topWall = false;
                }
                else if (dir.first == 0 && dir.second == 1) {
                    m_cells[row][col].rightWall = false;
                    m_cells[newRow][newCol].leftWall = false;
                }
                else if (dir.first == 0 && dir.second == -1) {
                    m_cells[row][col].leftWall = false;
                    m_cells[newRow][newCol].rightWall = false;
                }
                carveMaze(newRow, newCol);
            }
        }
    }
}

bool Maze::hasTopWall(int row, int col) const {
    return m_cells[row][col].topWall;
}
bool Maze::hasRightWall(int row, int col) const {
    return m_cells[row][col].rightWall;
}
bool Maze::hasBottomWall(int row, int col) const {
    return m_cells[row][col].bottomWall;
}
bool Maze::hasLeftWall(int row, int col) const {
    return m_cells[row][col].leftWall;
}

const Cell& Maze::getCell(int row, int col) const {
    return m_cells[row][col];
}

bool Maze::hasWall(int row, int col, Direction direction) const {
    switch (direction) {
    case Direction::Up:    return hasTopWall(row, col);
    case Direction::Right: return hasRightWall(row, col);
    case Direction::Down:  return hasBottomWall(row, col);
    case Direction::Left:  return hasLeftWall(row, col);
    }
    return true; // fallback na wypadek błędów — uznajemy, że jest ściana
}
