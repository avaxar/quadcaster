#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "grid_tree.hpp"


const SDL_Color GRID_PALETTE[] = {
    {0, 0, 0, 0},         // 0 (none)
    {192, 224, 255, 255}, // 1 (white)
    {255, 128, 128, 255}, // 2 (red)
    {128, 255, 128, 255}, // 3 (green)
    {128, 128, 255, 255}, // 4 (blue)
    {255, 255, 128, 255}, // 5 (yellow)
    {128, 255, 255, 255}, // 6 (cyan)
    {255, 128, 255, 255}  // 7 (purple)
};

class GridMap {
private:
    std::vector<std::vector<uint8_t>> map;

public:
    size_t width = 0;
    size_t height = 0;

    GridMap() {}
    GridMap(const std::string& file_name);

    uint8_t getAt(size_t x, size_t y) const;
    void setAt(size_t x, size_t y, uint8_t type);

    GridTree treeify() const;

private:
    void subtreeify(GridTree& tree, size_t x_start, size_t y_start, size_t size) const;
};
