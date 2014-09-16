#include <math.h>
#include <stdio.h>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#define MOD(a, b) (((a) % (b)) + (b)) % (b)

#define SCR_WIDTH 600
#define SCR_HEIGHT 600
#define GRD_WIDTH 15
#define GRD_HEIGHT 15
#define GRD_SPAWN 6

#define BLK_EMPTY 0
#define BLK_WALL 1
#define BLK_GLASS 2
#define BLK_POWERUP 3
#define BLK_GLASSPOWER 4

// probability of each block type, out of 1000
#define PRB_WALL 200
#define PRB_GLASS 100
#define PRB_POWERUP 5
#define PRB_GLASSPOWER 2

#define CLR_BACKGROUND sf::Color(128, 192, 255)

const sf::Color blkColours[] = {
    CLR_BACKGROUND,          // empty (not explicitly drawn)
    sf::Color::Black,        // wall
    sf::Color(32, 128, 192), // glass
    sf::Color(255, 32, 64),  // power-up
    sf::Color(128, 128, 192) // power-up inside glass
};

void genLine(int line[GRD_WIDTH], bool spawnSpace) {
    for (int x = 1; x < GRD_WIDTH - 1; x++) {
        // leave space under the spawn platform hole
        if (spawnSpace && (x > 5 && x < GRD_WIDTH - 6)) {line[x] = BLK_EMPTY; continue;}
        // select block at random, use probability weighting
        int blk = BLK_EMPTY;
        int prob = rand() % 1000;
        if (prob < PRB_WALL) blk = BLK_WALL;
        else if (prob < PRB_GLASS + PRB_WALL) blk = BLK_GLASS;
        else if (prob < PRB_POWERUP + PRB_WALL + PRB_GLASS) blk = BLK_POWERUP;
        else if (prob < PRB_GLASSPOWER + PRB_WALL + PRB_GLASS + PRB_POWERUP) blk = BLK_GLASSPOWER;
        line[x] = blk;
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(SCR_WIDTH, SCR_HEIGHT), "Deep Drop", sf::Style::Titlebar | sf::Style::Close);
    sf::Clock clock;
    int grid[GRD_HEIGHT + 1][GRD_WIDTH] = {BLK_EMPTY};
    // wall on left/right edges
    for (int y = 0; y < GRD_HEIGHT + 1; y++) {
        grid[y][0] = BLK_WALL;
        grid[y][GRD_WIDTH - 1] = BLK_WALL;
    }
    // spawn platform/roof
    for (int x = 1; x < GRD_WIDTH - 1; x++) {
        grid[0][x] = BLK_WALL;
        if (x < 6 || x > GRD_WIDTH - 7) grid[GRD_SPAWN][x] = BLK_WALL;
    }
    // random generation
    for (int y = GRD_SPAWN + 1; y < GRD_HEIGHT + 1; y++) {
        genLine(grid[y], y < GRD_SPAWN + 4);
    }
    /*
    +-----------------+
    |#################| # wall
    |#               #| ? random
    |#               #|
    |#               #|
    |#               #|
    |#               #|
    |#######   #######|
    |#??????   ??????#|
    |#??????   ??????#|
    |#??????   ??????#|
    |#???????????????#|
    |       ...       |
    +-----------------+
    */
    int lastOffset = 0;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                default:
                    break;
            }
        }
        window.clear(CLR_BACKGROUND);
        int blockWidth = SCR_WIDTH / GRD_WIDTH;
        int blockHeight = SCR_HEIGHT / GRD_HEIGHT;
        int offset = MOD(static_cast<int>(round((clock.getElapsedTime().asSeconds() / 1.5) * blockHeight)), blockHeight);
        // reached end of grid, need a new bottom row
        if (offset < lastOffset) {
            // shift all rows up (discard top row)
            for (int y = 0; y < GRD_HEIGHT; y++) {
                for (int x = 0; x < GRD_WIDTH; x++) {
                    grid[y][x] = grid[y + 1][x];
                }
            }
            genLine(grid[GRD_HEIGHT], false);
        }
        // draw grid
        for (int y = 0; y < GRD_HEIGHT + 1; y++) {
            for (int x = 0; x < GRD_WIDTH; x++) {
                // no need to draw empty cell
                if (grid[y][x] == BLK_EMPTY) continue;
                int width = blockWidth;
                int height = blockHeight;
                int left = x * width;
                int top = (y * height) - offset;
                sf::Color colour = blkColours[grid[y][x]];
                // power-ups are small squares in the middle of their cell
                if (grid[y][x] == BLK_POWERUP) {
                    width /= 3;
                    height /= 3;
                    left += blockWidth / 3;
                    top += blockHeight / 3;
                    // blink a lighter colour
                    if (MOD(clock.getElapsedTime().asMilliseconds() / 250, 2) == 1) colour = colour + sf::Color(32, 32, 32);
                }
                sf::RectangleShape rect(sf::Vector2f(width, height));
                rect.setPosition(left, top);
                rect.setFillColor(colour);
                window.draw(rect);
            }
        }
        lastOffset = offset;
        window.display();
    }
    return 0;
}
