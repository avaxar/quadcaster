#include <fstream>
#include <iostream>

#include "grid_map.hpp"
#include "utils.hpp"


GridMap::GridMap(const std::string& file_name) {
    // Loads file
    std::ifstream fstr(file_name);
    if (!fstr.is_open()) {
        std::cerr << "Failed to open `" << file_name << "`\n";
        return;
    }

    // Reads per line in the file
    std::string line;
    while (std::getline(fstr, line)) {
        // Updates width to the maximum line length
        if (line.size() > this->width) {
            this->width = line.size();
        }

        this->map.push_back({});
        this->map.back().reserve(line.size());

        // Reads per character in the line
        for (char c : line) {
            // Maps decimal numbers
            if ('0' <= c && c <= '9') {
                this->map.back().push_back(c - '0');
            }
            // Assumes zero if an unrecognized character is found
            else {
                this->map.back().push_back(0);
            }
        }
    }

    this->height = this->map.size();
}

uint8_t GridMap::getAt(size_t x, size_t y) const {
    if (y >= this->map.size()) {
        return 0;
    }

    if (x >= this->map[y].size()) {
        return 0;
    }

    return this->map[y][x];
}

void GridMap::setAt(size_t x, size_t y, uint8_t type) {
    // In case it attempts to set out of the current rows
    this->map.reserve(y + 1);
    while (y >= this->map.size()) {
        this->map.push_back({}); // Filler
    }

    // In case it attempts to set out of the current column of the row
    this->map[y].reserve(x + 1);
    while (x >= this->map[y].size()) {
        this->map[y].push_back(0); // Filler
    }

    // The actual assignment
    this->map[y][x] = type;

    // Updates width and height upon any prior size readjustments
    this->height = this->map.size();
    this->width = x > width ? x : width;
}

GridTree GridMap::treeify() const {
    size_t max_size = nextPowerOfTwo(this->width > this->height ? width : height);
    GridTree root;

    subtreeify(root, 0, 0, max_size);
    return root;
}

void GridMap::subtreeify(GridTree& tree, size_t x_start, size_t y_start, size_t size) const {
    // Instantiates a leaf, as it corresponds to a single block in the grid
    if (size == 1) {
        tree.color = GRID_PALETTE[this->getAt(x_start, y_start)];
        return;
    }

    // Fills the quadrants with subgrids for later assignments
    for (int i = 0b00; i <= 0b11; i++) {
        tree.setQuadrant(i & 0b01, i & 0b10, GridTree());
    }

#define REMOVE_IF_NULL(X, Y)                      \
    {                                             \
        auto& quad = tree.getQuadrant(X, Y);      \
        if (quad.isLeaf() && quad.color.a == 0) { \
            tree.clearQuadrant(X, Y);             \
        }                                         \
    }

    // Quadrant X- Y-
    this->subtreeify(tree.getQuadrant(false, false), x_start, y_start + size / 2, size / 2);
    REMOVE_IF_NULL(false, false);

    // Quadrant X- Y+
    this->subtreeify(tree.getQuadrant(false, true), x_start, y_start, size / 2);
    REMOVE_IF_NULL(false, true);

    // Quadrant X+ Y-
    this->subtreeify(tree.getQuadrant(true, false), x_start + size / 2, y_start + size / 2, size / 2);
    REMOVE_IF_NULL(true, false);

    // Quadrant X+ Y+
    this->subtreeify(tree.getQuadrant(true, true), x_start + size / 2, y_start, size / 2);
    REMOVE_IF_NULL(true, true);

    // Culls for homogeneous subgrids
    tree.cull();
}
