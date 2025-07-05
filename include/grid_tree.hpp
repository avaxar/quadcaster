#pragma once

#include <string>

#include <SDL3/SDL.h>


struct RayHit {
    bool hit;
    SDL_FPoint locus;
    SDL_Color color = {0, 0, 0, 0};
};

class GridTree {
private:
    /*
     * =====[Quadrants of the grid tree]=====
     *
     *          (Y+)
     *           |                 _(0)_
     *        1  |  0            /   |   \
     * (X-) -----+----- (X+)    /   / \   \
     *        2  |  3         (1) (2) (3) (4)
     *           |
     *          (Y-)
     */
    GridTree* quadrants[4] = {nullptr, nullptr, nullptr, nullptr};

public:
    SDL_Color color;

    GridTree(SDL_Color color = {0, 0, 0, 0}) : color(color) {}
    GridTree(const GridTree& orig);
    GridTree(GridTree&& orig);
    GridTree& operator=(GridTree&& orig);

    ~GridTree();

    void setQuadrant(bool xPos, bool yPos, const GridTree& tree);
    void clearQuadrant(bool xPos, bool yPos);
    bool hasQuadrant(bool xPos, bool yPos) const;
    GridTree& getQuadrant(bool xPos, bool yPos);
    bool isLeaf() const;

    void prune();
    RayHit cast(SDL_FPoint origin, float angle) const;
    std::string graphviz() const;

private:
    std::string graphviz(int& i) const;
};
