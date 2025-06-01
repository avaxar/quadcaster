#include "grid_tree.hpp"
#include "utils.hpp"


GridTree::GridTree(const GridTree& orig) {
    this->color = orig.color;
    for (int i = 0; i < 4; i++) {
        if (orig.quadrants[i]) {
            this->quadrants[i] = new GridTree(*orig.quadrants[i]);
        }
    }
}

GridTree::GridTree(GridTree&& orig) {
    this->color = orig.color;
    for (int i = 0; i < 4; i++) {
        this->quadrants[i] = orig.quadrants[i];
        orig.quadrants[i] = nullptr;
    }
}

GridTree& GridTree::operator=(GridTree&& orig) {
    if (this != &orig) {
        this->color = orig.color;
        for (int i = 0; i < 4; ++i) {
            delete this->quadrants[i];
            this->quadrants[i] = orig.quadrants[i];
            orig.quadrants[i] = nullptr;
        }
    }

    return *this;
}

GridTree::~GridTree() {
    for (int i = 0; i < 4; i++) {
        delete this->quadrants[i];
    }
}

void GridTree::setQuadrant(bool xPos, bool yPos, const GridTree& tree) {
    size_t index = mapQuadrantIndex(xPos, yPos);
    GridTree* old = this->quadrants[index];
    this->quadrants[index] = new GridTree(tree);

    if (old) {
        delete old;
    }
}

void GridTree::clearQuadrant(bool xPos, bool yPos) {
    size_t index = mapQuadrantIndex(xPos, yPos);
    if (this->quadrants[index]) {
        delete this->quadrants[index];
        this->quadrants[index] = nullptr;
    }
}

bool GridTree::hasQuadrant(bool xPos, bool yPos) const {
    return this->quadrants[mapQuadrantIndex(xPos, yPos)];
}

GridTree& GridTree::getQuadrant(bool xPos, bool yPos) {
    return *this->quadrants[mapQuadrantIndex(xPos, yPos)];
}

bool GridTree::isLeaf() const {
    for (int i = 0; i < 4; i++) {
        if (this->quadrants[i]) {
            return false;
        }
    }
    return true;
}

void GridTree::cull() {
    // Ensures that every quadrant exists and is a leaf
    for (int i = 0; i < 4; i++) {
        if (!this->quadrants[i] || !this->quadrants[i]->isLeaf()) {
            return;
        }
    }

    // Compares every quadrant leaf to have the same color
    SDL_Color color = this->quadrants[0]->color;
    for (int i = 1; i < 4; i++) {
        if (this->quadrants[i]->color.r != color.r || this->quadrants[i]->color.g != color.g ||
            this->quadrants[i]->color.b != color.b || this->quadrants[i]->color.a != color.a) {
            return;
        }
    }

    // Culls homogeneous quadrants
    for (int i = 0b00; i <= 0b11; i++) {
        this->clearQuadrant(i & 0b01, i & 0b10);
    }

    // Represents the overall color
    this->color = color;
}


RayHit GridTree::cast(SDL_FPoint origin, float angle) const {
    float x = origin.x;
    float y = origin.y;

    // Vector of ray angle
    float dx = std::sin(angle);
    float dy = std::cos(angle);

    // Performs DDA for anything exceeding the left boundary
    if (x < -1.0) {
        float d = (-1.0 - x) / dx;
        if (dx == 0.0 || d < 0.0) {
            return RayHit{.hit = false, .locus = origin};
        }

        x = -1.0;
        y += d * dy;
    }

    // Performs DDA for anything exceeding the right boundary
    if (x > 1.0) {
        float d = (1.0 - x) / dx;
        if (dx == 0.0 || d < 0.0) {
            return RayHit{.hit = false, .locus = origin};
        }

        x = 1.0;
        y += d * dy;
    }

    // Performs DDA for anything exceeding the bottom boundary
    if (y < -1.0) {
        float d = (-1.0 - y) / dy;
        if (dy == 0.0 || d < 0.0) {
            return RayHit{.hit = false, .locus = origin};
        }

        x += d * dx;
        y = -1.0;
    }

    // Performs DDA for anything exceeding the top boundary
    if (y > 1.0) {
        float d = (1.0 - y) / dy;
        if (dy == 0.0 || d < 0.0) {
            return RayHit{.hit = false, .locus = origin};
        }

        x += d * dx;
        y = 1.0;
    }

    // If it's inside a leaf tree, confirms ray hit success
    if (this->isLeaf()) {
        SDL_Color color = this->color;

        bool isAtRight = x + y > 0 && x - y > 0;
        bool isAtLeft = x + y < 0 && x - y < 0;

        // Ambient shading for the sides facing X+ and X-
        if (isAtRight || isAtLeft) {
            color.r = (uint8_t)(color.r * 0.95);
            color.g = (uint8_t)(color.g * 0.95);
            // Blue is left untouched for a colder color.
        }

        return RayHit{.hit = true, .locus = SDL_FPoint{x, y}, .color = color};
    }

    // Loops while still being in the bounds of the grid
    while (-1.0 <= x && x <= 1.0 && -1.0 <= y && y <= 1.0) {
        // Defines the bounds of the current subgrid relative to the grid's coordinates
        float x_min, x_max, y_min, y_max;
        if (x >= 0.0) {
            if (y >= 0.0) {
                x_min = 0.0;
                x_max = 1.0;
                y_min = 0.0;
                y_max = 1.0;
            }
            else {
                x_min = 0.0;
                x_max = 1.0;
                y_min = -1.0;
                y_max = 0.0;
            }
        }
        else {
            if (y >= 0.0) {
                x_min = -1.0;
                x_max = 0.0;
                y_min = 0.0;
                y_max = 1.0;
            }
            else {
                x_min = -1.0;
                x_max = 0.0;
                y_min = -1.0;
                y_max = 0.0;
            }
        }

        // If a tree is present in the subgrid
        if (GridTree* quadrant = this->quadrants[mapQuadrantIndex(x >= 0.0, y >= 0.0)]) {
            // Maps the current grid coordinates to the local subgrid coordinates
            SDL_FPoint local;
            local.x = remap(x, x_min, x_max, -1.0, 1.0);
            local.y = remap(y, y_min, y_max, -1.0, 1.0);

            // Recurses into the subgrid
            RayHit ray = quadrant->cast(local, angle);

            // Maps back the local subgrid coordinates to the current grid coordinates
            x = ray.locus.x = remap(ray.locus.x, -1.0, 1.0, x_min, x_max);
            y = ray.locus.y = remap(ray.locus.y, -1.0, 1.0, y_min, y_max);

            // Finally returns upon a successful ray hit
            if (ray.hit) {
                return ray;
            }
        }
        // If no tree is in the subgrid, projects a line shooting through the other side of the subgrid
        else {
            // Angles relative to (x, y) for each vertex of the empty grid
            float top_left = std::atan2(x_min - x, y_max - y);
            float top_right = std::atan2(x_max - x, y_max - y);
            float bottom_left = std::atan2(x_min - x, y_min - y);
            float bottom_right = std::atan2(x_max - x, y_min - y);

            // If projecting to the top side
            if (betweenAngle(angle, top_left, top_right)) {
                // DDA for the perpendicular X axis
                x += (y_max - y) / dy * dx;

                // Nudges inside, ensuring it's within the bounds of the top neighbor
                y = y_max + EPSILON;
            }
            // If projecting to the right side
            else if (betweenAngle(angle, top_right, bottom_right)) {
                // DDA for the perpendicular Y axis
                y += (x_max - x) / dx * dy;

                // Nudges inside, ensuring it's within the bounds of the right neighbor
                x = x_max + EPSILON;
            }
            // If projecting to the bottom side
            else if (betweenAngle(angle, bottom_right, bottom_left)) {
                // DDA for the perpendicular X axis
                x += (y_min - y) / dy * dx;

                // Nudges inside, ensuring it's within the bounds of the bottom neighbor
                y = y_min - EPSILON;
            }
            // If projecting to the left side
            else if (betweenAngle(angle, bottom_left, top_left)) {
                // DDA for the perpendicular Y axis
                y += (x_min - x) / dx * dy;

                // Nudges inside, ensuring it's within the bounds of the left neighbor
                x = x_min - EPSILON;
            }
        }
    }

    // Exits the grid tree without hitting anything
    return RayHit{.hit = false, .locus = SDL_FPoint{x, y}};
}

std::string GridTree::graphviz() const {
    int i = 0;
    return "digraph QuadTree {\n"
           "\tnode [shape=circle, style=filled, fontname=\"Helvetica\"];\n"
           "\n"
           "\tnode0 [label=\"Root\", fillcolor=\"black\", fontcolor=\"white\"];\n" +
           this->graphviz(i) + "}\n";
}

std::string GridTree::graphviz(int& i) const {
    int parent = i;
    std::string parent_str = std::to_string(parent);
    std::string out;

#define PRINT_QUADRANT(X, Y, NAME)                                                           \
    {                                                                                        \
        i++;                                                                                 \
        GridTree* quadrant = this->quadrants[mapQuadrantIndex(X, Y)];                        \
                                                                                             \
        std::string fill = "000000FF";                                                       \
        std::string font = "red";                                                            \
        if (quadrant) {                                                                      \
            fill = "000000FF";                                                               \
            font = "white";                                                                  \
                                                                                             \
            if (quadrant->isLeaf()) {                                                        \
                fill = toColorHex(quadrant->color);                                          \
                font = "black";                                                              \
            }                                                                                \
        }                                                                                    \
                                                                                             \
        out += "\tnode" + std::to_string(i) + " [label=\"" NAME "\", fillcolor=\"#" + fill + \
               "\", fontcolor=\"" + font + "\"];\n";                                         \
        out += "\tnode" + parent_str + " -> node" + std::to_string(i) + ";\n";               \
                                                                                             \
        if (quadrant) {                                                                      \
            out += quadrant->graphviz(i);                                                    \
        }                                                                                    \
    }

    if (!this->isLeaf()) {
        PRINT_QUADRANT(true, true, "X+ Y+");
        PRINT_QUADRANT(false, true, "X- Y+");
        PRINT_QUADRANT(false, false, "X- Y-");
        PRINT_QUADRANT(true, false, "X+ Y-");
    }

    return out;
}
