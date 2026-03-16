#include "DinoGame.h"
#include "HAL.h"
#include <cstdlib>
#include <cstdio>

DinoGame dinoGameApp(HAL::buttonManager());


DinoGame dinoGame;

void DinoGame::resetGame() {
    dinoY = GROUND_Y - DINO_H;
    dinoVelY = 0;
    isJumping = false;
    isDucking = false;
    isGameOver = false;
    score = 0;
    gameSpeed = 3.0f;
    lastUpdate = millis();
    lastObstacle = millis();
    lastScoreInc = millis();
    groundOffset = 0;
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].active = false;
    }
}

void DinoGame::begin() {
    highScore = 0;
    resetGame();
}

void DinoGame::spawnObstacle() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) {
            obstacles[i].x = 128;
            obstacles[i].width = 6 + random() % 8;
            obstacles[i].height = 10 + random() % 12;
            obstacles[i].active = true;
            break;
        }
    }
}

bool DinoGame::checkCollision() {
    int dinoX = 20;
    int dinoTop = (int)dinoY;
    int dinoRight = dinoX + DINO_W - 4;
    int dinoBottom = dinoTop + DINO_H;
    
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        int ox = (int)obstacles[i].x;
        int oy = GROUND_Y - obstacles[i].height;
        if (dinoRight > ox && dinoX < ox + obstacles[i].width &&
            dinoBottom > oy && dinoTop < GROUND_Y) {
            return true;
        }
    }
    return false;
}

void DinoGame::drawDino() {
    auto& display = HAL::displayProxy();
    int x = 20;
    int y = (int)dinoY;
    // Simple dino: body rectangle + head + eye + legs
    display.fillRect(x + 2, y, 10, 12);       // body
    display.fillRect(x + 6, y - 4, 8, 6);     // head
    display.setPixel(x + 12, y - 2);           // eye (clear pixel)
    // Legs (animate based on ground offset)
    if (isJumping) {
        display.drawLine(x + 3, y + 12, x + 1, y + 15);
        display.drawLine(x + 9, y + 12, x + 11, y + 15);
    } else if (groundOffset % 8 < 4) {
        display.drawLine(x + 3, y + 12, x + 1, y + 15);
        display.drawLine(x + 9, y + 12, x + 9, y + 15);
    } else {
        display.drawLine(x + 3, y + 12, x + 3, y + 15);
        display.drawLine(x + 9, y + 12, x + 11, y + 15);
    }
}

void DinoGame::drawObstacles() {
    auto& display = HAL::displayProxy();
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        int x = (int)obstacles[i].x;
        int y = GROUND_Y - obstacles[i].height;
        display.fillRect(x, y, obstacles[i].width, obstacles[i].height);
    }
}

void DinoGame::drawGround() {
    auto& display = HAL::displayProxy();
    display.drawLine(0, GROUND_Y, 127, GROUND_Y);
    // Dashed ground detail
    for (int x = -groundOffset % 12; x < 128; x += 12) {
        display.drawLine(x, GROUND_Y + 2, x + 4, GROUND_Y + 2);
    }
}

void DinoGame::drawScore() {
    auto& display = HAL::displayProxy();
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", score);
    display.drawString(90, 2, buf);
    if (highScore > 0) {
        snprintf(buf, sizeof(buf), "HI %d", highScore);
        display.drawString(50, 2, buf);
    }
}

void DinoGame::drawGameOver() {
    auto& display = HAL::displayProxy();
    display.drawString(30, 20, "GAME OVER");
    display.drawString(20, 34, "Press any button");
}

void DinoGame::update() {
    unsigned long now = millis();
    float dt = (now - lastUpdate) / 16.67f; // Normalize to ~60fps
    lastUpdate = now;
    
    auto& display = HAL::displayProxy();
    
    // Handle input
    ButtonEvent event;
    while (HAL::buttonManager().getNextEvent(event)) {
        if (isGameOver) {
            if (event.eventType == ButtonEvent_Pressed) {
                resetGame();
                return;
            }
        } else {
            if (event.eventType == ButtonEvent_Pressed && !isJumping) {
                isJumping = true;
                dinoVelY = JUMP_FORCE;
                HAL::audioManager().playTone(800, 30);
            }
        }
    }
    
    if (isGameOver) {
        display.clear();
        drawGround();
        drawDino();
        drawObstacles();
        drawScore();
        drawGameOver();
        display.display();
        delay(30);
        return;
    }
    
    // Physics
    dinoVelY += GRAVITY * dt;
    dinoY += dinoVelY * dt;
    if (dinoY >= GROUND_Y - DINO_H) {
        dinoY = GROUND_Y - DINO_H;
        dinoVelY = 0;
        isJumping = false;
    }
    
    // Move obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        obstacles[i].x -= gameSpeed * dt;
        if (obstacles[i].x < -20) {
            obstacles[i].active = false;
        }
    }
    
    // Spawn obstacles
    if (now - lastObstacle > (unsigned long)(1500 / (gameSpeed / 3.0f))) {
        spawnObstacle();
        lastObstacle = now;
    }
    
    // Score
    if (now - lastScoreInc > 100) {
        score++;
        lastScoreInc = now;
        if (score % 100 == 0) {
            gameSpeed += 0.3f;
            HAL::audioManager().playTone(1200, 50);
        }
    }
    
    // Collision
    if (checkCollision()) {
        isGameOver = true;
        if (score > highScore) highScore = score;
        HAL::audioManager().playTone(200, 200);
    }
    
    // Scroll ground
    groundOffset += (int)(gameSpeed * dt);
    
    // Draw
    display.clear();
    drawGround();
    drawDino();
    drawObstacles();
    drawScore();
    display.display();
    delay(16);
}

void DinoGame::end() {}