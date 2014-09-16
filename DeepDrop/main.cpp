#include "header.h"

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
#define SQR_WIDTH (SCR_WIDTH / GRD_WIDTH)
#define SQR_HEIGHT (SCR_HEIGHT / GRD_HEIGHT)
#define PLR_WIDTH (SQR_WIDTH / 3)
#define PLR_HEIGHT (SQR_HEIGHT / 2)

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
    // add player
    Player player;
    player.x = SQR_WIDTH * 3;
    player.y = SQR_HEIGHT * 6;
    /*
    +-----------------+
    |#################| # wall
    |#               #| P player
    |#               #| ? random
    |#               #|
    |#               #|
    |#  P            #|
    |#######   #######|
    |#??????   ??????#|
    |#??????   ??????#|
    |#??????   ??????#|
    |#???????????????#|
    |       ...       |
    +-----------------+
    */
    int lastTime = 0;
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
        // quit on Ctrl-C
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C)) {
            window.close();
            break;
        }
        // do movement based on arrow keys
        int diff = clock.getElapsedTime().asMilliseconds() - lastTime;
        lastTime += diff;
        bool mvLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
        bool mvRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
        if (mvLeft && !mvRight) {
            player.x -= (diff / 5);
            // reset to right edge of wall
            int x = player.x + 1;
            int y1 = player.y - 1;
            int y2 = y1 - PLR_HEIGHT;
            int cell1 = grid[y1 / SQR_HEIGHT][x / SQR_WIDTH];
            int cell2 = grid[y2 / SQR_HEIGHT][x / SQR_WIDTH];
            if (cell1 == BLK_WALL || cell1 == BLK_GLASS || cell1 == BLK_GLASSPOWER || cell2 == BLK_WALL || cell2 == BLK_GLASS || cell2 == BLK_GLASSPOWER) {
                player.x = ((x / SQR_WIDTH) + 1) * SQR_WIDTH;
            }
        } else if (mvRight && !mvLeft) {
            player.x += (diff / 5);
            // reset to left edge of wall
            int x = player.x + PLR_WIDTH + 1;
            int y1 = player.y - 1;
            int y2 = y1 - PLR_HEIGHT;
            int cell1 = grid[y1 / SQR_HEIGHT][x / SQR_WIDTH];
            int cell2 = grid[y2 / SQR_HEIGHT][x / SQR_WIDTH];
            if (cell1 == BLK_WALL || cell1 == BLK_GLASS || cell1 == BLK_GLASSPOWER || cell2 == BLK_WALL || cell2 == BLK_GLASS || cell2 == BLK_GLASSPOWER) {
                player.x = ((x / SQR_WIDTH) * SQR_WIDTH) - PLR_WIDTH;
            }
        }
        // generate a new bottom row if scrolled to that point
        int offset = MOD(static_cast<int>(round((clock.getElapsedTime().asSeconds() / 1.5) * SQR_HEIGHT)), SQR_HEIGHT);
        if (offset < lastOffset) {
            // shift all rows up (discard top row)
            for (int y = 0; y < GRD_HEIGHT; y++) {
                for (int x = 0; x < GRD_WIDTH; x++) {
                    grid[y][x] = grid[y + 1][x];
                }
            }
            player.y -= SQR_HEIGHT;
            genLine(grid[GRD_HEIGHT], false);
        }
        // draw grid
        window.clear(CLR_BACKGROUND);
        for (int y = 0; y < GRD_HEIGHT + 1; y++) {
            for (int x = 0; x < GRD_WIDTH; x++) {
                // no need to draw empty cell
                if (grid[y][x] == BLK_EMPTY) continue;
                int width = SQR_WIDTH;
                int height = SQR_HEIGHT;
                int left = x * width;
                int top = (y * height) - offset;
                sf::Color colour = blkColours[grid[y][x]];
                // power-ups are small squares in the middle of their cell
                if (grid[y][x] == BLK_POWERUP) {
                    width /= 3;
                    height /= 3;
                    left += SQR_WIDTH / 3;
                    top += SQR_HEIGHT / 3;
                    // blink a lighter colour
                    if (MOD(clock.getElapsedTime().asMilliseconds() / 250, 2) == 1) colour = colour + sf::Color(32, 32, 32);
                }
                sf::RectangleShape rect(sf::Vector2f(width, height));
                rect.setPosition(left, top);
                rect.setFillColor(colour);
                window.draw(rect);
            }
        }
        sf::RectangleShape playerRect(sf::Vector2f(PLR_WIDTH, PLR_HEIGHT));
        playerRect.setPosition(player.x, player.y - PLR_HEIGHT - offset);
        playerRect.setFillColor(sf::Color(64, 32, 0));
        window.draw(playerRect);
        lastOffset = offset;
        window.display();
    }
    return 0;
}
