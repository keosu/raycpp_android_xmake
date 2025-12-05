#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include <raylib-cpp/raylib-cpp.hpp>

// Game constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float PLAYER_SPEED = 5.0f;
const float BULLET_SPEED = 8.0f;
const float ENEMY_SPEED = 2.0f;
const int MAX_BULLETS = 50;
const int MAX_ENEMIES = 20;

// Game states
enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

// Particle system
struct Particle {
    raylib::Vector2 position;
    raylib::Vector2 velocity;
    raylib::Color color;
    float lifetime;
    float maxLifetime;
    float size;
    
    Particle(raylib::Vector2 pos, raylib::Vector2 vel, raylib::Color col, float life, float sz)
        : position(pos), velocity(vel), color(col), lifetime(life), maxLifetime(life), size(sz) {}
    
    void Update() {
        position.x += velocity.x;
        position.y += velocity.y;
        lifetime -= GetFrameTime();
        velocity.y += 0.1f; // Gravity
    }
    
    bool IsAlive() const { return lifetime > 0; }
    
    void Draw() const {
        float alpha = lifetime / maxLifetime;
        raylib::Color drawColor = color;
        drawColor.a = static_cast<unsigned char>(255 * alpha);
        DrawCircleV(position, size, drawColor);
    }
};

// Bullet class
struct Bullet {
    raylib::Vector2 position;
    raylib::Vector2 velocity;
    bool active;
    
    Bullet() : position(0, 0), velocity(0, 0), active(false) {}
    
    void Spawn(raylib::Vector2 pos, raylib::Vector2 vel) {
        position = pos;
        velocity = vel;
        active = true;
    }
    
    void Update() {
        if (active) {
            position.x += velocity.x;
            position.y += velocity.y;
            
            // Deactivate if off screen
            if (position.y < -10 || position.y > SCREEN_HEIGHT + 10 ||
                position.x < -10 || position.x > SCREEN_WIDTH + 10) {
                active = false;
            }
        }
    }
    
    void Draw() const {
        if (active) {
            DrawCircleV(position, 4, YELLOW);
            DrawCircleV(position, 2, WHITE);
        }
    }
};

// Enemy class
struct Enemy {
    raylib::Vector2 position;
    raylib::Vector2 velocity;
    bool active;
    int health;
    float rotation;
    raylib::Color color;
    
    Enemy() : position(0, 0), velocity(0, 0), active(false), health(1), rotation(0), color(RED) {}
    
    void Spawn(raylib::Vector2 pos, raylib::Vector2 vel, int hp) {
        position = pos;
        velocity = vel;
        active = true;
        health = hp;
        rotation = 0;
        color = (hp > 1) ? PURPLE : RED;
    }
    
    void Update() {
        if (active) {
            position.x += velocity.x;
            position.y += velocity.y;
            rotation += 2.0f;
            
            // Deactivate if off screen
            if (position.y > SCREEN_HEIGHT + 50) {
                active = false;
            }
        }
    }
    
    void Draw() const {
        if (active) {
            // Draw rotating enemy ship
            raylib::Vector2 v1(position.x, position.y - 15);
            raylib::Vector2 v2(position.x - 12, position.y + 12);
            raylib::Vector2 v3(position.x + 12, position.y + 12);
            
            // Rotate points
            float rad = rotation * DEG2RAD;
            float cosR = cosf(rad);
            float sinR = sinf(rad);
            
            auto rotatePoint = [&](raylib::Vector2 p) -> raylib::Vector2 {
                float x = p.x - position.x;
                float y = p.y - position.y;
                return {
                    position.x + x * cosR - y * sinR,
                    position.y + x * sinR + y * cosR
                };
            };
            
            v1 = rotatePoint(v1);
            v2 = rotatePoint(v2);
            v3 = rotatePoint(v3);
            
            DrawTriangle(v1, v2, v3, color);
            DrawTriangleLines(v1, v2, v3, DARKGRAY);
            
            // Draw health indicator
            if (health > 1) {
                DrawCircle(position.x, position.y, 3, ORANGE);
            }
        }
    }
    
    bool CheckCollision(const Bullet& bullet) const {
        if (!active || !bullet.active) return false;
        return CheckCollisionCircles(position, 15, bullet.position, 4);
    }
};

// Player class
struct Player {
    raylib::Vector2 position;
    int health;
    int score;
    float shootCooldown;
    bool invincible;
    float invincibleTimer;
    
    Player() {
        position = raylib::Vector2(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT - 80.0f);
        health = 5;
        score = 0;
        shootCooldown = 0;
        invincible = false;
        invincibleTimer = 0;
    }
    
    void Update() {
        // Movement
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) position.x -= PLAYER_SPEED;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) position.x += PLAYER_SPEED;
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) position.y -= PLAYER_SPEED;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) position.y += PLAYER_SPEED;
        
#ifdef PLATFORM_ANDROID
        // Touch input for mobile
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            raylib::Vector2 touch = GetMousePosition();
            raylib::Vector2 direction(touch.x - position.x, touch.y - position.y);
            float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
            if (length > 50) {
                direction.x = (direction.x / length) * PLAYER_SPEED;
                direction.y = (direction.y / length) * PLAYER_SPEED;
                position.x += direction.x;
                position.y += direction.y;
            }
        }
#endif
        
        // Keep player on screen
        if (position.x < 30) position.x = 30;
        if (position.x > SCREEN_WIDTH - 30) position.x = SCREEN_WIDTH - 30;
        if (position.y < 30) position.y = 30;
        if (position.y > SCREEN_HEIGHT - 30) position.y = SCREEN_HEIGHT - 30;
        
        // Update cooldowns
        if (shootCooldown > 0) shootCooldown -= GetFrameTime();
        if (invincibleTimer > 0) {
            invincibleTimer -= GetFrameTime();
            if (invincibleTimer <= 0) invincible = false;
        }
    }
    
    bool CanShoot() const {
        return shootCooldown <= 0;
    }
    
    void Shoot() {
        shootCooldown = 0.15f;
    }
    
    void TakeDamage() {
        if (!invincible) {
            health--;
            invincible = true;
            invincibleTimer = 2.0f;
        }
    }
    
    void Draw() const {
        // Draw player ship
        raylib::Color shipColor = invincible ? BLUE : SKYBLUE;
        if (invincible && (int)(invincibleTimer * 10) % 2 == 0) {
            shipColor.a = 128;
        }
        
        // Main body
        DrawTriangle(
            raylib::Vector2(position.x, position.y - 20),
            raylib::Vector2(position.x - 15, position.y + 15),
            raylib::Vector2(position.x + 15, position.y + 15),
            shipColor
        );
        
        // Cockpit
        DrawCircle(position.x, position.y, 6, DARKBLUE);
        
        // Wings
        DrawRectangle(position.x - 20, position.y + 5, 8, 12, shipColor);
        DrawRectangle(position.x + 12, position.y + 5, 8, 12, shipColor);
        
        // Engine glow
        DrawCircle(position.x - 10, position.y + 15, 3, ORANGE);
        DrawCircle(position.x + 10, position.y + 15, 3, ORANGE);
    }
    
    bool CheckCollision(const Enemy& enemy) const {
        if (!enemy.active || invincible) return false;
        return CheckCollisionCircles(position, 15, enemy.position, 15);
    }
};

// Particle manager
class ParticleManager {
private:
    std::vector<Particle> particles;
    
public:
    void AddExplosion(raylib::Vector2 position, raylib::Color color) {
        for (int i = 0; i < 20; i++) {
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float speed = (float)GetRandomValue(2, 6);
            raylib::Vector2 velocity(cosf(angle) * speed, sinf(angle) * speed);
            float size = (float)GetRandomValue(2, 5);
            particles.emplace_back(position, velocity, color, 1.0f, size);
        }
    }
    
    void AddTrail(raylib::Vector2 position, raylib::Color color) {
        for (int i = 0; i < 2; i++) {
            raylib::Vector2 velocity(
                (float)GetRandomValue(-10, 10) / 10.0f,
                (float)GetRandomValue(10, 30) / 10.0f
            );
            particles.emplace_back(position, velocity, color, 0.5f, 2.0f);
        }
    }
    
    void Update() {
        for (auto& p : particles) {
            p.Update();
        }
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [](const Particle& p) { return !p.IsAlive(); }),
            particles.end()
        );
    }
    
    void Draw() const {
        for (const auto& p : particles) {
            p.Draw();
        }
    }
    
    void Clear() {
        particles.clear();
    }
};

// Main game class
class SpaceShooter {
private:
    GameState state;
    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    ParticleManager particles;
    float enemySpawnTimer;
    float difficultyTimer;
    int wave;
    
public:
    SpaceShooter() {
        state = MENU;
        bullets.resize(MAX_BULLETS);
        enemies.resize(MAX_ENEMIES);
        enemySpawnTimer = 0;
        difficultyTimer = 0;
        wave = 1;
    }
    
    void Reset() {
        player = Player();
        for (auto& b : bullets) b.active = false;
        for (auto& e : enemies) e.active = false;
        particles.Clear();
        enemySpawnTimer = 0;
        difficultyTimer = 0;
        wave = 1;
        state = PLAYING;
    }
    
    void Update() {
        switch (state) {
            case MENU:
                if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || 
                    IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Reset();
                }
                break;
                
            case PLAYING:
                UpdateGame();
                break;
                
            case PAUSED:
                if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
                    state = PLAYING;
                }
                break;
                
            case GAME_OVER:
                if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
                    IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    state = MENU;
                }
                break;
        }
    }
    
    void UpdateGame() {
        // Check pause
        if (IsKeyPressed(KEY_P)) {
            state = PAUSED;
            return;
        }
        
        // Update player
        player.Update();
        
        // Player shooting
        if ((IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_LEFT_BUTTON)) && 
            player.CanShoot()) {
            SpawnBullet(player.position, {0, -BULLET_SPEED});
            player.Shoot();
        }
        
        // Update bullets
        for (auto& bullet : bullets) {
            bullet.Update();
        }
        
        // Spawn enemies
        enemySpawnTimer += GetFrameTime();
        float spawnRate = std::max(0.5f, 2.0f - difficultyTimer / 30.0f);
        if (enemySpawnTimer >= spawnRate) {
            enemySpawnTimer = 0;
            SpawnEnemy();
        }
        
        // Update difficulty
        difficultyTimer += GetFrameTime();
        wave = 1 + (int)(difficultyTimer / 20.0f);
        
        // Update enemies
        for (auto& enemy : enemies) {
            enemy.Update();
            
            // Add engine trail
            if (enemy.active && GetRandomValue(0, 5) == 0) {
                particles.AddTrail(raylib::Vector2(enemy.position.x, enemy.position.y + 10), RED);
            }
        }
        
        // Add player engine trail
        if (GetRandomValue(0, 2) == 0) {
            particles.AddTrail(raylib::Vector2(player.position.x - 10, player.position.y + 15), ORANGE);
            particles.AddTrail(raylib::Vector2(player.position.x + 10, player.position.y + 15), ORANGE);
        }
        
        // Collision detection
        CheckCollisions();
        
        // Update particles
        particles.Update();
        
        // Check game over
        if (player.health <= 0) {
            state = GAME_OVER;
        }
    }
    
    void SpawnBullet(raylib::Vector2 pos, raylib::Vector2 vel) {
        for (auto& bullet : bullets) {
            if (!bullet.active) {
                bullet.Spawn(pos, vel);
                break;
            }
        }
    }
    
    void SpawnEnemy() {
        float x = (float)GetRandomValue(50, SCREEN_WIDTH - 50);
        int health = (GetRandomValue(0, 100) < 20 + wave * 5) ? 2 : 1;
        float speedMultiplier = 1.0f + difficultyTimer / 60.0f;
        
        for (auto& enemy : enemies) {
            if (!enemy.active) {
                enemy.Spawn(
                    raylib::Vector2(x, -30),
                    raylib::Vector2(0, ENEMY_SPEED * speedMultiplier),
                    health
                );
                break;
            }
        }
    }
    
    void CheckCollisions() {
        // Bullet-Enemy collisions
        for (auto& bullet : bullets) {
            if (!bullet.active) continue;
            
            for (auto& enemy : enemies) {
                if (enemy.CheckCollision(bullet)) {
                    bullet.active = false;
                    enemy.health--;
                    
                    if (enemy.health <= 0) {
                        particles.AddExplosion(enemy.position, enemy.color);
                        player.score += 10;
                        enemy.active = false;
                    }
                    break;
                }
            }
        }
        
        // Player-Enemy collisions
        for (auto& enemy : enemies) {
            if (player.CheckCollision(enemy)) {
                particles.AddExplosion(enemy.position, RED);
                particles.AddExplosion(player.position, BLUE);
                player.TakeDamage();
                enemy.active = false;
            }
        }
    }
    
    void Draw() {
        ClearBackground(BLACK);
        
        // Draw starfield background
        DrawStarfield();
        
        switch (state) {
            case MENU:
                DrawMenu();
                break;
                
            case PLAYING:
                DrawGame();
                break;
                
            case PAUSED:
                DrawGame();
                DrawPaused();
                break;
                
            case GAME_OVER:
                DrawGame();
                DrawGameOver();
                break;
        }
    }
    
    void DrawStarfield() {
        static float starOffset = 0;
        starOffset += 0.5f;
        if (starOffset > SCREEN_HEIGHT) starOffset = 0;
        
        for (int i = 0; i < 100; i++) {
            int x = (i * 97) % SCREEN_WIDTH;
            int y = (int)fmodf((float)(i * 67) + starOffset, (float)SCREEN_HEIGHT);
            int brightness = 100 + (i * 13) % 156;
            raylib::Color starColor((unsigned char)brightness, (unsigned char)brightness, 
                                    (unsigned char)brightness, 255);
            DrawPixel(x, y, starColor);
        }
    }
    
    void DrawMenu() {
        // Title
        DrawText("SPACE DEFENDER", SCREEN_WIDTH/2 - 220, 150, 60, SKYBLUE);
        DrawText("SPACE DEFENDER", SCREEN_WIDTH/2 - 222, 148, 60, BLUE);
        
        // Instructions
        const char* instruction = "PRESS SPACE TO START";
#ifdef PLATFORM_ANDROID
        instruction = "TAP TO START";
#endif
        DrawText(instruction, SCREEN_WIDTH/2 - 180, 300, 30, WHITE);
        
        // Controls
        DrawText("CONTROLS:", SCREEN_WIDTH/2 - 100, 380, 20, YELLOW);
#ifdef PLATFORM_ANDROID
        DrawText("Touch to move", SCREEN_WIDTH/2 - 100, 410, 16, WHITE);
        DrawText("Auto shoot", SCREEN_WIDTH/2 - 100, 435, 16, WHITE);
#else
        DrawText("WASD or Arrow Keys - Move", SCREEN_WIDTH/2 - 150, 410, 16, WHITE);
        DrawText("SPACE - Shoot", SCREEN_WIDTH/2 - 150, 435, 16, WHITE);
        DrawText("P - Pause", SCREEN_WIDTH/2 - 150, 460, 16, WHITE);
#endif
        
        // Animated ship
        float time = GetTime();
        DrawTriangle(
            raylib::Vector2(SCREEN_WIDTH/2.0f, 240 + sinf(time * 2) * 10),
            raylib::Vector2(SCREEN_WIDTH/2.0f - 15, 275 + sinf(time * 2) * 10),
            raylib::Vector2(SCREEN_WIDTH/2.0f + 15, 275 + sinf(time * 2) * 10),
            SKYBLUE
        );
    }
    
    void DrawGame() {
        // Draw game objects
        particles.Draw();
        
        for (const auto& bullet : bullets) {
            bullet.Draw();
        }
        
        for (const auto& enemy : enemies) {
            enemy.Draw();
        }
        
        player.Draw();
        
        // Draw UI
        DrawUI();
    }
    
    void DrawUI() {
        // Score
        DrawText(TextFormat("SCORE: %d", player.score), 10, 10, 20, YELLOW);
        
        // Wave
        DrawText(TextFormat("WAVE: %d", wave), 10, 35, 16, SKYBLUE);
        
        // Health
        DrawText("HEALTH:", SCREEN_WIDTH - 180, 10, 20, RED);
        for (int i = 0; i < player.health; i++) {
            DrawRectangle(SCREEN_WIDTH - 180 + 90 + i * 18, 13, 15, 15, RED);
        }
        
        // FPS
        DrawFPS(SCREEN_WIDTH - 80, SCREEN_HEIGHT - 25);
    }
    
    void DrawPaused() {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 0, 0, 180});
        DrawText("PAUSED", SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 - 40, 60, WHITE);
        DrawText("Press P to continue", SCREEN_WIDTH/2 - 130, SCREEN_HEIGHT/2 + 40, 20, LIGHTGRAY);
    }
    
    void DrawGameOver() {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 0, 0, 180});
        DrawText("GAME OVER", SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT/2 - 80, 60, RED);
        DrawText(TextFormat("Final Score: %d", player.score), 
                 SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 + 20, 30, YELLOW);
        DrawText(TextFormat("Wave Reached: %d", wave),
                 SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2 + 60, 25, SKYBLUE);
                 
        const char* restart = "Press SPACE to return to menu";
#ifdef PLATFORM_ANDROID
        restart = "Tap to return to menu";
#endif
        DrawText(restart, SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 + 120, 20, WHITE);
    }
};

int main() {
    // Initialize window
    raylib::Window window(SCREEN_WIDTH, SCREEN_HEIGHT, "Space Defender");
    SetTargetFPS(60);
    
#ifdef PLATFORM_ANDROID
    std::cout << "Space Defender - Android Version" << std::endl;
#else
    std::cout << "Space Defender - Desktop Version" << std::endl;
    std::cout << "Controls: WASD/Arrows to move, SPACE to shoot, P to pause" << std::endl;
#endif
    
    // Initialize game
    SpaceShooter game;
    
    // Main game loop
    while (!window.ShouldClose()) {
        // Update
        game.Update();
        
        // Draw
        window.BeginDrawing();
        game.Draw();
        window.EndDrawing();
    }
    
    std::cout << "Thanks for playing!" << std::endl;
    return 0;
}