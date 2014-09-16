#include <math.h>
#include <stdio.h>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#define MOD(a, b) (((a) % (b)) + (b)) % (b)

#define SCR_WIDTH 600
#define SCR_HEIGHT 600
#define GRD_WIDTH 15
#define GRD_HEIGHT 15

#define BLK_EMPTY 0
#define BLK_WALL 1
#define BLK_GLASS 2

int main() {
    sf::RenderWindow window(sf::VideoMode(SCR_WIDTH, SCR_HEIGHT), "Deep Drop", sf::Style::Titlebar | sf::Style::Close);
    sf::Clock clock;
    int grid[GRD_HEIGHT + 1][GRD_WIDTH] = {BLK_EMPTY};
    for (int y = 0; y < GRD_HEIGHT + 1; y++) {
        grid[y][0] = BLK_WALL;
        grid[y][GRD_WIDTH - 1] = BLK_WALL;
        int count = (rand() % 5) + 1;
        for (int i = 0; i < count; i++) {
            grid[y][(rand() % (GRD_WIDTH - 2)) + 1] = (rand() % 5 < 2) ? BLK_GLASS : BLK_WALL;
        }
    }
    sf::Color colours[3] = {sf::Color(128, 192, 255), sf::Color::Black, sf::Color(32, 128, 192)};
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
        window.clear(sf::Color::Black);
        int blockWidth = SCR_WIDTH / GRD_WIDTH;
        int blockHeight = SCR_HEIGHT / GRD_HEIGHT;
        int offset = MOD(static_cast<int>(round((clock.getElapsedTime().asSeconds() / 1.5) * blockHeight)), blockHeight);
        if (offset < lastOffset) {
            for (int y = 0; y < GRD_HEIGHT; y++) {
                for (int x = 0; x < GRD_WIDTH; x++) {
                    grid[y][x] = grid[y + 1][x];
                }
            }
            for (int x = 1; x < GRD_WIDTH - 1; x++) {
                grid[GRD_HEIGHT][x] = BLK_EMPTY;
            }
            int count = (rand() % 5) + 1;
            for (int i = 0; i < count; i++) {
                grid[GRD_HEIGHT][(rand() % (GRD_WIDTH - 2)) + 1] = (rand() % 5 < 2) ? BLK_GLASS : BLK_WALL;
            }
        }
        for (int y = 0; y < GRD_HEIGHT + 1; y++) {
            for (int x = 0; x < GRD_WIDTH; x++) {
                sf::RectangleShape rect(sf::Vector2f(blockWidth, blockHeight));
                rect.setPosition(x * blockWidth, (y * blockHeight) - offset);
                rect.setFillColor(colours[grid[y][x]]);
                window.draw(rect);
            }
        }
        lastOffset = offset;
        window.display();
    }
    return 0;
}
