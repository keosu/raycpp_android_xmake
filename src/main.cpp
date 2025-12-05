#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include <raylib-cpp/raylib-cpp.hpp>

// Screen helper functions
inline int GetGameWidth() { return GetScreenWidth(); }
inline int GetGameHeight() { return GetScreenHeight(); }
inline bool IsPortrait() { return GetScreenHeight() > GetScreenWidth(); }
inline bool IsLandscape() { return GetScreenWidth() >= GetScreenHeight(); }

// Game constants - will be scaled based on screen size
inline float GetScaleFactor() {
    float baseWidth = IsPortrait() ? 600.0f : 800.0f;
    return GetGameWidth() / baseWidth;
}

const float BASE_PLAYER_SPEED = 5.0f;
const float BASE_BULLET_SPEED = 8.0f;
const float BASE_ENEMY_SPEED = 2.0f;
const int MAX_BULLETS = 50;
const int MAX_ENEMIES = 20;

// Dynamic game values
inline float GetPlayerSpeed() { return BASE_PLAYER_SPEED * GetScaleFactor(); }
inline float GetBulletSpeed() { return BASE_BULLET_SPEED * GetScaleFactor(); }
inline float GetEnemySpeed() { return BASE_ENEMY_SPEED * GetScaleFactor(); }

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
            if (position.y < -10 || position.y > GetGameHeight() + 10 ||
                position.x < -10 || position.x > GetGameWidth() + 10) {
                active = false;
            }
        }
    }
    
    void Draw() const {
        if (active) {
            float scale = GetScaleFactor();
            DrawCircleV(position, 4 * scale, YELLOW);
            DrawCircleV(position, 2 * scale, WHITE);
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
            if (position.y > GetGameHeight() + 50 * GetScaleFactor()) {
                active = false;
            }
        }
    }
    
    void Draw() const {
        if (active) {
            float scale = GetScaleFactor();
            // Draw rotating enemy ship
            raylib::Vector2 v1(position.x, position.y - 15 * scale);
            raylib::Vector2 v2(position.x - 12 * scale, position.y + 12 * scale);
            raylib::Vector2 v3(position.x + 12 * scale, position.y + 12 * scale);
            
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
                DrawCircle(position.x, position.y, 3 * scale, ORANGE);
            }
        }
    }
    
    bool CheckCollision(const Bullet& bullet) const {
        if (!active || !bullet.active) return false;
        float scale = GetScaleFactor();
        return CheckCollisionCircles(position, 15 * scale, bullet.position, 4 * scale);
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
        position = raylib::Vector2(GetGameWidth() / 2.0f, GetGameHeight() - 80.0f * GetScaleFactor());
        health = 5;
        score = 0;
        shootCooldown = 0;
        invincible = false;
        invincibleTimer = 0;
    }
    
    void Reset() {
        position = raylib::Vector2(GetGameWidth() / 2.0f, GetGameHeight() - 80.0f * GetScaleFactor());
        health = 5;
        score = 0;
        shootCooldown = 0;
        invincible = false;
        invincibleTimer = 0;
    }
    
    void Update() {
        float speed = GetPlayerSpeed();
        
        // Movement
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) position.x -= speed;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) position.x += speed;
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) position.y -= speed;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) position.y += speed;
        
#ifdef PLATFORM_ANDROID
        // Touch input for mobile
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            raylib::Vector2 touch = GetMousePosition();
            raylib::Vector2 direction(touch.x - position.x, touch.y - position.y);
            float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
            float minDistance = 50 * GetScaleFactor();
            if (length > minDistance) {
                direction.x = (direction.x / length) * speed;
                direction.y = (direction.y / length) * speed;
                position.x += direction.x;
                position.y += direction.y;
            }
        }
#endif
        
        // Keep player on screen
        float margin = 30 * GetScaleFactor();
        if (position.x < margin) position.x = margin;
        if (position.x > GetGameWidth() - margin) position.x = GetGameWidth() - margin;
        if (position.y < margin) position.y = margin;
        if (position.y > GetGameHeight() - margin) position.y = GetGameHeight() - margin;
        
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
        float scale = GetScaleFactor();
        
        // Draw player ship
        raylib::Color shipColor = invincible ? BLUE : SKYBLUE;
        if (invincible && (int)(invincibleTimer * 10) % 2 == 0) {
            shipColor.a = 128;
        }
        
        // Main body
        DrawTriangle(
            raylib::Vector2(position.x, position.y - 20 * scale),
            raylib::Vector2(position.x - 15 * scale, position.y + 15 * scale),
            raylib::Vector2(position.x + 15 * scale, position.y + 15 * scale),
            shipColor
        );
        
        // Cockpit
        DrawCircle(position.x, position.y, 6 * scale, DARKBLUE);
        
        // Wings
        DrawRectangle(position.x - 20 * scale, position.y + 5 * scale, 8 * scale, 12 * scale, shipColor);
        DrawRectangle(position.x + 12 * scale, position.y + 5 * scale, 8 * scale, 12 * scale, shipColor);
        
        // Engine glow
        DrawCircle(position.x - 10 * scale, position.y + 15 * scale, 3 * scale, ORANGE);
        DrawCircle(position.x + 10 * scale, position.y + 15 * scale, 3 * scale, ORANGE);
    }
    
    bool CheckCollision(const Enemy& enemy) const {
        if (!enemy.active || invincible) return false;
        float scale = GetScaleFactor();
        return CheckCollisionCircles(position, 15 * scale, enemy.position, 15 * scale);
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
        player.Reset();
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
            SpawnBullet(player.position, raylib::Vector2(0, -GetBulletSpeed()));
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
                float scale = GetScaleFactor();
                particles.AddTrail(raylib::Vector2(enemy.position.x, enemy.position.y + 10 * scale), RED);
            }
        }
        
        // Add player engine trail
        if (GetRandomValue(0, 2) == 0) {
            float scale = GetScaleFactor();
            particles.AddTrail(raylib::Vector2(player.position.x - 10 * scale, player.position.y + 15 * scale), ORANGE);
            particles.AddTrail(raylib::Vector2(player.position.x + 10 * scale, player.position.y + 15 * scale), ORANGE);
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
        float margin = 50 * GetScaleFactor();
        float x = (float)GetRandomValue(margin, GetGameWidth() - margin);
        int health = (GetRandomValue(0, 100) < 20 + wave * 5) ? 2 : 1;
        float speedMultiplier = 1.0f + difficultyTimer / 60.0f;
        
        for (auto& enemy : enemies) {
            if (!enemy.active) {
                enemy.Spawn(
                    raylib::Vector2(x, -30 * GetScaleFactor()),
                    raylib::Vector2(0, GetEnemySpeed() * speedMultiplier),
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
        starOffset += 0.5f * GetScaleFactor();
        if (starOffset > GetGameHeight()) starOffset = 0;
        
        int starCount = (int)(100 * GetScaleFactor());
        for (int i = 0; i < starCount; i++) {
            int x = (i * 97) % GetGameWidth();
            int y = (int)fmodf((float)(i * 67) + starOffset, (float)GetGameHeight());
            int brightness = 100 + (i * 13) % 156;
            raylib::Color starColor((unsigned char)brightness, (unsigned char)brightness, 
                                    (unsigned char)brightness, 255);
            DrawPixel(x, y, starColor);
        }
    }
    
    void DrawMenu() {
        int centerX = GetGameWidth() / 2;
        float scale = GetScaleFactor();
        int titleSize = (int)(60 * scale);
        int instructionSize = (int)(30 * scale);
        int textSize = (int)(16 * scale);
        
        // Title
        const char* title = "SPACE DEFENDER";
        int titleWidth = MeasureText(title, titleSize);
        DrawText(title, centerX - titleWidth/2, (int)(150 * scale), titleSize, SKYBLUE);
        DrawText(title, centerX - titleWidth/2 - 2, (int)(148 * scale), titleSize, BLUE);
        
        // Instructions
        const char* instruction = "PRESS SPACE TO START";
#ifdef PLATFORM_ANDROID
        instruction = "TAP TO START";
#endif
        int instrWidth = MeasureText(instruction, instructionSize);
        DrawText(instruction, centerX - instrWidth/2, (int)(300 * scale), instructionSize, WHITE);
        
        // Controls
        const char* controlsTitle = "CONTROLS:";
        int controlsTitleWidth = MeasureText(controlsTitle, (int)(20 * scale));
        DrawText(controlsTitle, centerX - controlsTitleWidth/2, (int)(380 * scale), (int)(20 * scale), YELLOW);
        
#ifdef PLATFORM_ANDROID
        int y = (int)(410 * scale);
        const char* touch = "Touch to move";
        const char* shoot = "Auto shoot";
        int touchWidth = MeasureText(touch, textSize);
        int shootWidth = MeasureText(shoot, textSize);
        DrawText(touch, centerX - touchWidth/2, y, textSize, WHITE);
        DrawText(shoot, centerX - shootWidth/2, y + (int)(25 * scale), textSize, WHITE);
#else
        int y = (int)(410 * scale);
        const char* move = "WASD or Arrow Keys - Move";
        const char* shoot = "SPACE - Shoot";
        const char* pause = "P - Pause";
        int moveWidth = MeasureText(move, textSize);
        int shootWidth = MeasureText(shoot, textSize);
        int pauseWidth = MeasureText(pause, textSize);
        DrawText(move, centerX - moveWidth/2, y, textSize, WHITE);
        DrawText(shoot, centerX - shootWidth/2, y + (int)(25 * scale), textSize, WHITE);
        DrawText(pause, centerX - pauseWidth/2, y + (int)(50 * scale), textSize, WHITE);
#endif
        
        // Animated ship
        float time = GetTime();
        float shipY = 240 * scale + sinf(time * 2) * 10 * scale;
        DrawTriangle(
            raylib::Vector2(centerX, shipY),
            raylib::Vector2(centerX - 15 * scale, shipY + 35 * scale),
            raylib::Vector2(centerX + 15 * scale, shipY + 35 * scale),
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
        float scale = GetScaleFactor();
        int scoreSize = (int)(20 * scale);
        int waveSize = (int)(16 * scale);
        int margin = (int)(10 * scale);
        
        // Score
        DrawText(TextFormat("SCORE: %d", player.score), margin, margin, scoreSize, YELLOW);
        
        // Wave
        DrawText(TextFormat("WAVE: %d", wave), margin, margin + scoreSize + 5, waveSize, SKYBLUE);
        
        // Health
        int healthX = GetGameWidth() - (int)(180 * scale);
        DrawText("HEALTH:", healthX, margin, scoreSize, RED);
        for (int i = 0; i < player.health; i++) {
            DrawRectangle(healthX + (int)(90 * scale) + i * (int)(18 * scale), 
                         margin + (int)(3 * scale), 
                         (int)(15 * scale), (int)(15 * scale), RED);
        }
        
        // FPS (smaller on mobile)
#ifdef PLATFORM_ANDROID
        if (IsLandscape()) {
            DrawFPS(GetGameWidth() - (int)(80 * scale), GetGameHeight() - (int)(25 * scale));
        }
#else
        DrawFPS(GetGameWidth() - (int)(80 * scale), GetGameHeight() - (int)(25 * scale));
#endif
    }
    
    void DrawPaused() {
        DrawRectangle(0, 0, GetGameWidth(), GetGameHeight(), {0, 0, 0, 180});
        int centerX = GetGameWidth() / 2;
        int centerY = GetGameHeight() / 2;
        float scale = GetScaleFactor();
        
        const char* paused = "PAUSED";
        int pausedSize = (int)(60 * scale);
        int pausedWidth = MeasureText(paused, pausedSize);
        DrawText(paused, centerX - pausedWidth/2, centerY - (int)(40 * scale), pausedSize, WHITE);
        
        const char* cont = "Press P to continue";
        int contSize = (int)(20 * scale);
        int contWidth = MeasureText(cont, contSize);
        DrawText(cont, centerX - contWidth/2, centerY + (int)(40 * scale), contSize, LIGHTGRAY);
    }
    
    void DrawGameOver() {
        DrawRectangle(0, 0, GetGameWidth(), GetGameHeight(), {0, 0, 0, 180});
        int centerX = GetGameWidth() / 2;
        int centerY = GetGameHeight() / 2;
        float scale = GetScaleFactor();
        
        const char* gameOver = "GAME OVER";
        int gameOverSize = (int)(60 * scale);
        int gameOverWidth = MeasureText(gameOver, gameOverSize);
        DrawText(gameOver, centerX - gameOverWidth/2, centerY - (int)(80 * scale), gameOverSize, RED);
        
        const char* finalScore = TextFormat("Final Score: %d", player.score);
        int scoreSize = (int)(30 * scale);
        int scoreWidth = MeasureText(finalScore, scoreSize);
        DrawText(finalScore, centerX - scoreWidth/2, centerY + (int)(20 * scale), scoreSize, YELLOW);
        
        const char* waveText = TextFormat("Wave Reached: %d", wave);
        int waveSize = (int)(25 * scale);
        int waveWidth = MeasureText(waveText, waveSize);
        DrawText(waveText, centerX - waveWidth/2, centerY + (int)(60 * scale), waveSize, SKYBLUE);
                 
        const char* restart = "Press SPACE to return to menu";
#ifdef PLATFORM_ANDROID
        restart = "Tap to return to menu";
#endif
        int restartSize = (int)(20 * scale);
        int restartWidth = MeasureText(restart, restartSize);
        DrawText(restart, centerX - restartWidth/2, centerY + (int)(120 * scale), restartSize, WHITE);
    }
};

int main() {
    // Initialize window
#ifdef PLATFORM_ANDROID
    // On Android, use device screen size
    raylib::Window window(0, 0, "Space Defender");
    // Enable full screen on Android
    // SetConfigFlags(FLAG_FULLSCREEN_MODE);
#else
    // On desktop, use fixed size
    raylib::Window window(800, 600, "Space Defender");
#endif
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