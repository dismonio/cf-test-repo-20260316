#ifndef DINOGAME_H
#define DINOGAME_H

#include "AppManager.h"

#define DINO_W 16
#define DINO_H 16
#define GROUND_Y 52
#define MAX_OBSTACLES 3
#define GRAVITY 0.6f
#define JUMP_FORCE -8.0f

class DinoGame : public App {
public:
    void begin() override;
    void update() override;
    void end() override;

private:
    // Dino state
    float dinoY;
    float dinoVelY;
    bool isJumping;
    bool isDucking;
    bool isGameOver;
    
    // Obstacles
    struct Obstacle {
        float x;
        int width;
        int height;
        bool active;
    };
    Obstacle obstacles[MAX_OBSTACLES];
    
    // Game state
    int score;
    int highScore;
    float gameSpeed;
    unsigned long lastUpdate;
    unsigned long lastObstacle;
    unsigned long lastScoreInc;
    int groundOffset;
    
    void spawnObstacle();
    void drawDino();
    void drawObstacles();
    void drawGround();
    void drawScore();
    void drawGameOver();
    bool checkCollision();
    void resetGame();
};

extern DinoGame dinoGame;

extern DinoGame dinoGameApp;

#endif