#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>
#include <deque>
#include <raylib-cpp/raylib-cpp.hpp>

// Screen helper functions
inline int GetGameWidth() { return GetScreenWidth(); }
inline int GetGameHeight() { return GetScreenHeight(); }
inline bool IsPortrait() { return GetScreenHeight() > GetScreenWidth(); }

inline float GetScaleFactor() {
    float baseWidth = IsPortrait() ? 600.0f : 800.0f;
    return GetGameWidth() / baseWidth;
}

// UI Constants
const int TAB_HEIGHT = 80;
const int CARD_MARGIN = 20;
const int CARD_PADDING = 15;
const int GRAPH_HISTORY = 100;

// Tabs for different sensor views
enum SensorTab {
    TAB_ACCELEROMETER = 0,
    TAB_GYROSCOPE,
    TAB_MAGNETOMETER,
    TAB_TOUCH,
    TAB_COMPASS,
    TAB_COUNT
};

const char* TAB_NAMES[] = {
    "ACCEL",
    "GYRO", 
    "MAG",
    "TOUCH",
    "COMPASS"
};

const char* TAB_ICONS[] = {
    "📱",  // Accelerometer
    "🔄",  // Gyroscope
    "🧭",  // Magnetometer
    "👆",  // Touch
    "🧭"   // Compass
};

// Utility functions
inline std::string FormatFloat(float value, int precision = 2) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

inline float LerpFloat(float a, float b, float t) {
    return a + (b - a) * t;
}

inline raylib::Color LerpColor(raylib::Color a, raylib::Color b, float t) {
    return raylib::Color(
        (unsigned char)LerpFloat(a.r, b.r, t),
        (unsigned char)LerpFloat(a.g, b.g, t),
        (unsigned char)LerpFloat(a.b, b.b, t),
        (unsigned char)LerpFloat(a.a, b.a, t)
    );
}

inline float ClampFloat(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Sensor data history for graphs
template<typename T>
class DataHistory {
private:
    std::deque<T> data;
    size_t maxSize;
    
public:
    DataHistory(size_t size = GRAPH_HISTORY) : maxSize(size) {}
    
    void Push(T value) {
        data.push_back(value);
        if (data.size() > maxSize) {
            data.pop_front();
        }
    }
    
    const std::deque<T>& GetData() const { return data; }
    
    T GetLatest() const {
        return data.empty() ? T{} : data.back();
    }
    
    void Clear() { data.clear(); }
    
    size_t Size() const { return data.size(); }
};

// Vector3 data history
class Vector3History {
public:
    DataHistory<float> x;
    DataHistory<float> y;
    DataHistory<float> z;
    
    Vector3History() : x(GRAPH_HISTORY), y(GRAPH_HISTORY), z(GRAPH_HISTORY) {}
    
    void Push(raylib::Vector3 vec) {
        x.Push(vec.x);
        y.Push(vec.y);
        z.Push(vec.z);
    }
    
    void Clear() {
        x.Clear();
        y.Clear();
        z.Clear();
    }
};

// Particle for visual effects
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
        velocity.x *= 0.98f;
        velocity.y *= 0.98f;
    }
    
    bool IsAlive() const { return lifetime > 0; }
    
    void Draw() const {
        float alpha = lifetime / maxLifetime;
        raylib::Color drawColor = color;
        drawColor.a = (unsigned char)(255 * alpha);
        DrawCircleV(position, size * alpha, drawColor);
    }
};

// Touch point tracking
struct TouchPoint {
    raylib::Vector2 position;
    raylib::Vector2 lastPosition;
    int id;
    float pressure;
    bool active;
    raylib::Color color;
    float lifetime;
    
    TouchPoint() : position(0, 0), lastPosition(0, 0), id(0), pressure(1.0f), 
                   active(false), color(SKYBLUE), lifetime(0) {}
};

// Main sensor demo app
class SensorDemoApp {
private:
    SensorTab currentTab;
    
    // Sensor data
    Vector3History accelHistory;
    Vector3History gyroHistory;
    Vector3History magHistory;
    
    // Current sensor values (smoothed)
    raylib::Vector3 accelValue;
    raylib::Vector3 gyroValue;
    raylib::Vector3 magValue;
    
    // Touch data
    std::vector<TouchPoint> touchPoints;
    std::vector<Particle> particles;
    
    // Compass
    float compassAngle;
    float compassTargetAngle;
    
    // UI state
    float tabTransition;
    int targetTab;
    
    // 3D visualization
    Camera3D camera;
    raylib::Vector3 cubeRotation;
    
public:
    SensorDemoApp() {
        currentTab = TAB_ACCELEROMETER;
        targetTab = TAB_ACCELEROMETER;
        tabTransition = 0.0f;
        
        accelValue = raylib::Vector3(0, 0, 0);
        gyroValue = raylib::Vector3(0, 0, 0);
        magValue = raylib::Vector3(0, 0, 0);
        
        compassAngle = 0.0f;
        compassTargetAngle = 0.0f;
        
        touchPoints.resize(10);
        
        cubeRotation = raylib::Vector3(0, 0, 0);
        
        // Setup 3D camera
        camera.position = raylib::Vector3(0.0f, 10.0f, 10.0f);
        camera.target = raylib::Vector3(0.0f, 0.0f, 0.0f);
        camera.up = raylib::Vector3(0.0f, 1.0f, 0.0f);
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;
    }
    
    void Update() {
        UpdateSensors();
        UpdateTouch();
        UpdateUI();
        UpdateParticles();
        Update3DVisualization();
    }
    
    void UpdateSensors() {
        // Read accelerometer (gravity + motion)
        // Note: Raylib's GetAccelerometerData() may not be available on all platforms
        raylib::Vector3 rawAccel(0, -1, 0);  // Default gravity
        
#ifdef PLATFORM_ANDROID
        // Try to get actual accelerometer data
        // On Android, this should work if sensors are available
        float ax = 0, ay = -1, az = 0;
        // Simulate tilt for now - in real implementation, use Android NDK sensor API
        float time = GetTime();
        ax = sinf(time * 0.5f) * 0.3f;
        ay = -1.0f + cosf(time * 0.7f) * 0.2f;
        az = sinf(time * 0.3f) * 0.2f;
        rawAccel = raylib::Vector3(ax, ay, az);
#else
        // Desktop simulation
        float time = GetTime();
        rawAccel.x = sinf(time * 0.5f) * 0.5f;
        rawAccel.y = -1.0f + cosf(time * 0.7f) * 0.3f;
        rawAccel.z = sinf(time * 0.3f) * 0.3f;
#endif
        
        accelValue.x = LerpFloat(accelValue.x, rawAccel.x, 0.1f);
        accelValue.y = LerpFloat(accelValue.y, rawAccel.y, 0.1f);
        accelValue.z = LerpFloat(accelValue.z, rawAccel.z, 0.1f);
        accelHistory.Push(accelValue);
        
        // Gyroscope data (simulated on desktop, real on Android)
#ifdef PLATFORM_ANDROID
        // On Android, use actual gyroscope if available
        // For this demo, we'll simulate based on accelerometer
        gyroValue.x = (accelValue.y - 0.5f) * 2.0f;
        gyroValue.y = (accelValue.x - 0.5f) * 2.0f;
        gyroValue.z = 0.0f;
#else
        // Simulate on desktop
        float time = GetTime();
        gyroValue.x = sinf(time * 0.5f) * 0.5f;
        gyroValue.y = cosf(time * 0.7f) * 0.5f;
        gyroValue.z = sinf(time * 0.3f) * 0.3f;
#endif
        gyroHistory.Push(gyroValue);
        
        // Magnetometer (simulated - would need NDK sensor API for real data)
        magValue.x = LerpFloat(magValue.x, sinf(GetTime() * 0.2f), 0.05f);
        magValue.y = LerpFloat(magValue.y, cosf(GetTime() * 0.3f), 0.05f);
        magValue.z = LerpFloat(magValue.z, sinf(GetTime() * 0.1f) * 0.5f, 0.05f);
        magHistory.Push(magValue);
        
        // Calculate compass heading from magnetometer
        compassTargetAngle = atan2f(magValue.y, magValue.x) * RAD2DEG;
        compassAngle = LerpFloat(compassAngle, compassTargetAngle, 0.05f);
    }
    
    void UpdateTouch() {
        // Update touch points
        for (int i = 0; i < 10; i++) {
            if (i < GetTouchPointCount()) {
                raylib::Vector2 pos = GetTouchPosition(i);
                
                if (!touchPoints[i].active) {
                    // New touch
                    touchPoints[i].position = pos;
                    touchPoints[i].lastPosition = pos;
                    touchPoints[i].active = true;
                    touchPoints[i].id = i;
                    touchPoints[i].lifetime = 1.0f;
                    touchPoints[i].color = raylib::Color(
                        GetRandomValue(100, 255),
                        GetRandomValue(100, 255),
                        GetRandomValue(100, 255),
                        255
                    );
                    
                    // Spawn particles
                    SpawnTouchParticles(pos, touchPoints[i].color);
                } else {
                    // Update existing touch
                    touchPoints[i].lastPosition = touchPoints[i].position;
                    touchPoints[i].position = pos;
                    touchPoints[i].lifetime = 1.0f;
                    
                    // Create trail particles
                    if (Vector2Distance(touchPoints[i].position, touchPoints[i].lastPosition) > 5) {
                        SpawnTrailParticles(touchPoints[i].position, touchPoints[i].color);
                    }
                }
            } else {
                if (touchPoints[i].active) {
                    // Touch ended
                    SpawnReleaseParticles(touchPoints[i].position, touchPoints[i].color);
                    touchPoints[i].active = false;
                }
                touchPoints[i].lifetime -= GetFrameTime();
            }
        }
    }
    
    void UpdateUI() {
        // Tab switching
        if (tabTransition > 0) {
            tabTransition -= GetFrameTime() * 5.0f;
            if (tabTransition <= 0) {
                currentTab = (SensorTab)targetTab;
                tabTransition = 0;
            }
        }
    }
    
    void UpdateParticles() {
        for (auto& p : particles) {
            p.Update();
        }
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [](const Particle& p) { return !p.IsAlive(); }),
            particles.end()
        );
    }
    
    void Update3DVisualization() {
        // Update cube rotation based on sensors
        cubeRotation.x += gyroValue.x * GetFrameTime() * 50.0f;
        cubeRotation.y += gyroValue.y * GetFrameTime() * 50.0f;
        cubeRotation.z += gyroValue.z * GetFrameTime() * 50.0f;
        
        // Apply accelerometer tilt to camera
        camera.target.x = accelValue.x * 2.0f;
        camera.target.y = -accelValue.z * 2.0f;
    }
    
    void SpawnTouchParticles(raylib::Vector2 pos, raylib::Color color) {
        for (int i = 0; i < 20; i++) {
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float speed = (float)GetRandomValue(2, 8);
            raylib::Vector2 vel(cosf(angle) * speed, sinf(angle) * speed);
            particles.emplace_back(pos, vel, color, 1.0f, GetRandomValue(3, 8) * GetScaleFactor());
        }
    }
    
    void SpawnTrailParticles(raylib::Vector2 pos, raylib::Color color) {
        for (int i = 0; i < 3; i++) {
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float speed = (float)GetRandomValue(1, 3);
            raylib::Vector2 vel(cosf(angle) * speed, sinf(angle) * speed);
            particles.emplace_back(pos, vel, color, 0.5f, GetRandomValue(2, 5) * GetScaleFactor());
        }
    }
    
    void SpawnReleaseParticles(raylib::Vector2 pos, raylib::Color color) {
        for (int i = 0; i < 30; i++) {
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float speed = (float)GetRandomValue(5, 15);
            raylib::Vector2 vel(cosf(angle) * speed, sinf(angle) * speed);
            particles.emplace_back(pos, vel, color, 1.5f, GetRandomValue(4, 10) * GetScaleFactor());
        }
    }
    
    void Draw() {
        ClearBackground(raylib::Color(15, 15, 25, 255));
        
        DrawParticles();
        DrawContent();
        DrawTabs();
        DrawHeader();
    }
    
    void DrawParticles() {
        for (const auto& p : particles) {
            p.Draw();
        }
    }
    
    void DrawContent() {
        int contentY = (int)(TAB_HEIGHT * 1.5f * GetScaleFactor());
        int contentHeight = GetGameHeight() - contentY;
        
        switch (currentTab) {
            case TAB_ACCELEROMETER:
                DrawAccelerometerView(contentY, contentHeight);
                break;
            case TAB_GYROSCOPE:
                DrawGyroscopeView(contentY, contentHeight);
                break;
            case TAB_MAGNETOMETER:
                DrawMagnetometerView(contentY, contentHeight);
                break;
            case TAB_TOUCH:
                DrawTouchView(contentY, contentHeight);
                break;
            case TAB_COMPASS:
                DrawCompassView(contentY, contentHeight);
                break;
            case TAB_COUNT:
                // Not a real tab
                break;
        }
    }
    
    void DrawAccelerometerView(int y, int height) {
        float scale = GetScaleFactor();
        int centerX = GetGameWidth() / 2;
        
        // Card background
        DrawCard(CARD_MARGIN, y + CARD_MARGIN, GetGameWidth() - CARD_MARGIN * 2, 200 * scale);
        
        // Title
        ::DrawText("ACCELEROMETER", centerX - 150 * scale, y + CARD_MARGIN + CARD_PADDING, 24 * scale, SKYBLUE);
        ::DrawText("Measures device acceleration + gravity", 
                 centerX - 180 * scale, y + CARD_MARGIN + CARD_PADDING + 30 * scale, 
                 14 * scale, LIGHTGRAY);
        
        // Values
        int valY = y + CARD_MARGIN + 80 * scale;
        DrawSensorValue("X", accelValue.x, centerX - 200 * scale, valY, RED);
        DrawSensorValue("Y", accelValue.y, centerX - 20 * scale, valY, GREEN);
        DrawSensorValue("Z", accelValue.z, centerX + 160 * scale, valY, BLUE);
        
        // 3D visualization
        Draw3DCube(y + 250 * scale, 300 * scale);
        
        // Graph
        DrawVectorGraph(accelHistory, y + 580 * scale, 250 * scale, "Acceleration History");
    }
    
    void DrawGyroscopeView(int y, int height) {
        float scale = GetScaleFactor();
        int centerX = GetGameWidth() / 2;
        
        DrawCard(CARD_MARGIN, y + CARD_MARGIN, GetGameWidth() - CARD_MARGIN * 2, 200 * scale);
        
        ::DrawText("GYROSCOPE", centerX - 120 * scale, y + CARD_MARGIN + CARD_PADDING, 24 * scale, ORANGE);
        ::DrawText("Measures rotation rate", 
                 centerX - 100 * scale, y + CARD_MARGIN + CARD_PADDING + 30 * scale, 
                 14 * scale, LIGHTGRAY);
        
        int valY = y + CARD_MARGIN + 80 * scale;
        DrawSensorValue("Pitch", gyroValue.x, centerX - 220 * scale, valY, PINK);
        DrawSensorValue("Roll", gyroValue.y, centerX - 40 * scale, valY, YELLOW);
        DrawSensorValue("Yaw", gyroValue.z, centerX + 140 * scale, valY, PURPLE);
        
        // Rotating wheel visualization
        DrawRotationWheel(centerX, y + 350 * scale, 150 * scale);
        
        DrawVectorGraph(gyroHistory, y + 580 * scale, 250 * scale, "Rotation Rate History");
    }
    
    void DrawMagnetometerView(int y, int height) {
        float scale = GetScaleFactor();
        int centerX = GetGameWidth() / 2;
        
        DrawCard(CARD_MARGIN, y + CARD_MARGIN, GetGameWidth() - CARD_MARGIN * 2, 200 * scale);
        
        ::DrawText("MAGNETOMETER", centerX - 150 * scale, y + CARD_MARGIN + CARD_PADDING, 24 * scale, VIOLET);
        ::DrawText("Measures magnetic field strength", 
                 centerX - 160 * scale, y + CARD_MARGIN + CARD_PADDING + 30 * scale, 
                 14 * scale, LIGHTGRAY);
        
        int valY = y + CARD_MARGIN + 80 * scale;
        DrawSensorValue("X", magValue.x, centerX - 200 * scale, valY, RED);
        DrawSensorValue("Y", magValue.y, centerX - 20 * scale, valY, GREEN);
        DrawSensorValue("Z", magValue.z, centerX + 160 * scale, valY, BLUE);
        
        // Magnetic field visualization
        DrawMagneticField(centerX, y + 350 * scale, 150 * scale);
        
        DrawVectorGraph(magHistory, y + 580 * scale, 250 * scale, "Magnetic Field History");
    }
    
    void DrawTouchView(int y, int height) {
        float scale = GetScaleFactor();
        int centerX = GetGameWidth() / 2;
        
        DrawCard(CARD_MARGIN, y + CARD_MARGIN, GetGameWidth() - CARD_MARGIN * 2, 120 * scale);
        
        ::DrawText("TOUCH INPUT", centerX - 120 * scale, y + CARD_MARGIN + CARD_PADDING, 24 * scale, GOLD);
        ::DrawText("Touch the screen to create particles!", 
                 centerX - 180 * scale, y + CARD_MARGIN + CARD_PADDING + 30 * scale, 
                 14 * scale, LIGHTGRAY);
        
        int touchCount = 0;
        for (const auto& tp : touchPoints) {
            if (tp.active) touchCount++;
        }
        ::DrawText(TextFormat("Active touches: %d", touchCount), 
                 centerX - 100 * scale, y + CARD_MARGIN + 75 * scale, 
                 18 * scale, WHITE);
        
        // Draw active touch points
        for (const auto& tp : touchPoints) {
            if (tp.active) {
                // Glow effect
                DrawCircleGradient(tp.position.x, tp.position.y, 50 * scale, 
                                   Fade(tp.color, 0.3f), Fade(tp.color, 0.0f));
                
                // Touch point
                DrawCircleV(tp.position, 30 * scale, Fade(tp.color, 0.5f));
                DrawCircleV(tp.position, 25 * scale, tp.color);
                DrawCircleLinesV(tp.position, 35 * scale, WHITE);
                
                // Touch ID
                ::DrawText(TextFormat("%d", tp.id), 
                         tp.position.x - 5 * scale, tp.position.y - 10 * scale, 
                         20 * scale, BLACK);
                
                // Trail line
                if (Vector2Distance(tp.position, tp.lastPosition) > 1) {
                    DrawLineEx(tp.lastPosition, tp.position, 3 * scale, Fade(tp.color, 0.7f));
                }
            }
        }
    }
    
    void DrawCompassView(int y, int height) {
        float scale = GetScaleFactor();
        int centerX = GetGameWidth() / 2;
        
        DrawCard(CARD_MARGIN, y + CARD_MARGIN, GetGameWidth() - CARD_MARGIN * 2, 150 * scale);
        
        ::DrawText("COMPASS", centerX - 100 * scale, y + CARD_MARGIN + CARD_PADDING, 24 * scale, LIME);
        ::DrawText("Based on magnetometer data", 
                 centerX - 140 * scale, y + CARD_MARGIN + CARD_PADDING + 30 * scale, 
                 14 * scale, LIGHTGRAY);
        
        ::DrawText(TextFormat("Heading: %.1f°", compassAngle), 
                 centerX - 80 * scale, y + CARD_MARGIN + 90 * scale, 
                 20 * scale, YELLOW);
        
        // Large compass
        DrawCompass(centerX, y + 350 * scale, 200 * scale);
    }
    
    void DrawCard(int x, int y, int width, int height) {
        // Shadow
        DrawRectangle(x + 5, y + 5, width, height, Fade(BLACK, 0.3f));
        // Card background
        DrawRectangleGradientV(x, y, width, height, 
                               raylib::Color(30, 30, 45, 255), 
                               raylib::Color(20, 20, 35, 255));
        // Border
        DrawRectangleLinesEx(Rectangle{(float)x, (float)y, (float)width, (float)height}, 
                            2, raylib::Color(60, 60, 80, 255));
    }
    
    void DrawSensorValue(const char* label, float value, int x, int y, raylib::Color color) {
        float scale = GetScaleFactor();
        
        // Label
        ::DrawText(label, x, y, 18 * scale, color);
        
        // Value
        std::string valStr = FormatFloat(value, 3);
        ::DrawText(valStr.c_str(), x, y + 25 * scale, 20 * scale, WHITE);
        
        // Bar indicator
        int barWidth = 150 * scale;
        int barHeight = 12 * scale;
        int barY = y + 55 * scale;
        
        DrawRectangle(x, barY, barWidth, barHeight, raylib::Color(40, 40, 60, 255));
        
        float normalized = (value + 2.0f) / 4.0f;
        normalized = ClampFloat(normalized, 0.0f, 1.0f);
        int fillWidth = (int)(barWidth * normalized);
        
        DrawRectangle(x, barY, fillWidth, barHeight, color);
        DrawRectangleLinesEx(Rectangle{(float)x, (float)barY, (float)barWidth, (float)barHeight}, 
                            1, color);
    }
    
    void Draw3DCube(int y, int height) {
        int centerX = GetGameWidth() / 2;
        float scale = GetScaleFactor();
        
        // 3D view area
        BeginMode3D(camera);
        
        // Apply rotations from sensors
        rlPushMatrix();
        rlRotatef(cubeRotation.x, 1, 0, 0);
        rlRotatef(cubeRotation.y, 0, 1, 0);
        rlRotatef(cubeRotation.z, 0, 0, 1);
        
        DrawCube(raylib::Vector3(0, 0, 0), 4.0f, 4.0f, 4.0f, SKYBLUE);
        DrawCubeWires(raylib::Vector3(0, 0, 0), 4.0f, 4.0f, 4.0f, DARKBLUE);
        
        // Axes
        DrawLine3D(raylib::Vector3(0, 0, 0), raylib::Vector3(3, 0, 0), RED);
        DrawLine3D(raylib::Vector3(0, 0, 0), raylib::Vector3(0, 3, 0), GREEN);
        DrawLine3D(raylib::Vector3(0, 0, 0), raylib::Vector3(0, 0, 3), BLUE);
        
        rlPopMatrix();
        
        DrawGrid(10, 1.0f);
        
        EndMode3D();
    }
    
    void DrawVectorGraph(const Vector3History& history, int y, int height, const char* title) {
        float scale = GetScaleFactor();
        int margin = CARD_MARGIN;
        int width = GetGameWidth() - margin * 2;
        
        DrawCard(margin, y, width, height);
        
        ::DrawText(title, margin + CARD_PADDING, y + CARD_PADDING, 16 * scale, LIGHTGRAY);
        
        int graphY = y + 40 * scale;
        int graphHeight = height - 60 * scale;
        int graphX = margin + CARD_PADDING;
        int graphWidth = width - CARD_PADDING * 2;
        
        // Background
        DrawRectangle(graphX, graphY, graphWidth, graphHeight, raylib::Color(20, 20, 30, 255));
        
        // Grid lines
        for (int i = 0; i <= 4; i++) {
            int lineY = graphY + (graphHeight * i) / 4;
            DrawLine(graphX, lineY, graphX + graphWidth, lineY, raylib::Color(40, 40, 50, 255));
        }
        
        DrawLine(graphX, graphY + graphHeight/2, graphX + graphWidth, graphY + graphHeight/2, 
                 raylib::Color(60, 60, 70, 255));
        
        // Draw lines
        DrawGraphLine(history.x.GetData(), graphX, graphY, graphWidth, graphHeight, RED);
        DrawGraphLine(history.y.GetData(), graphX, graphY, graphWidth, graphHeight, GREEN);
        DrawGraphLine(history.z.GetData(), graphX, graphY, graphWidth, graphHeight, BLUE);
        
        // Legend
        int legendX = graphX + graphWidth - 120 * scale;
        int legendY = graphY + 10 * scale;
        ::DrawText("X", legendX, legendY, 14 * scale, RED);
        ::DrawText("Y", legendX + 30 * scale, legendY, 14 * scale, GREEN);
        ::DrawText("Z", legendX + 60 * scale, legendY, 14 * scale, BLUE);
    }
    
    void DrawGraphLine(const std::deque<float>& data, int x, int y, int width, int height, raylib::Color color) {
        if (data.size() < 2) return;
        
        float xStep = (float)width / (GRAPH_HISTORY - 1);
        int centerY = y + height / 2;
        float scale = height / 6.0f;
        
        for (size_t i = 1; i < data.size(); i++) {
            float x1 = x + (i - 1) * xStep;
            float y1 = centerY - data[i-1] * scale;
            float x2 = x + i * xStep;
            float y2 = centerY - data[i] * scale;
            
            DrawLineEx(raylib::Vector2(x1, y1), raylib::Vector2(x2, y2), 2, color);
        }
    }
    
    void DrawRotationWheel(int centerX, int centerY, float radius) {
        float scale = GetScaleFactor();
        
        // Outer circle
        DrawCircleGradient(centerX, centerY, radius * 1.2f, 
                          Fade(ORANGE, 0.2f), Fade(ORANGE, 0.0f));
        DrawCircleLines(centerX, centerY, radius, ORANGE);
        
        // Inner wheel
        DrawCircle(centerX, centerY, radius * 0.9f, raylib::Color(30, 30, 45, 255));
        
        // Rotation indicators
        float angleX = gyroValue.x * 30;
        float angleY = gyroValue.y * 30;
        
        // X axis (pitch) - vertical line
        float x1 = centerX + sinf(angleX * DEG2RAD) * radius * 0.8f;
        float y1 = centerY - cosf(angleX * DEG2RAD) * radius * 0.8f;
        float x2 = centerX - sinf(angleX * DEG2RAD) * radius * 0.8f;
        float y2 = centerY + cosf(angleX * DEG2RAD) * radius * 0.8f;
        DrawLineEx(raylib::Vector2(x1, y1), raylib::Vector2(x2, y2), 4 * scale, PINK);
        
        // Y axis (roll) - horizontal line
        float x3 = centerX + cosf(angleY * DEG2RAD) * radius * 0.8f;
        float y3 = centerY + sinf(angleY * DEG2RAD) * radius * 0.8f;
        float x4 = centerX - cosf(angleY * DEG2RAD) * radius * 0.8f;
        float y4 = centerY - sinf(angleY * DEG2RAD) * radius * 0.8f;
        DrawLineEx(raylib::Vector2(x3, y3), raylib::Vector2(x4, y4), 4 * scale, YELLOW);
        
        // Center dot
        DrawCircle(centerX, centerY, 8 * scale, WHITE);
    }
    
    void DrawMagneticField(int centerX, int centerY, float radius) {
        float scale = GetScaleFactor();
        
        // Field lines
        for (int i = 0; i < 8; i++) {
            float angle = (i * 45) * DEG2RAD;
            float dist = radius * (0.5f + fabsf(sinf(GetTime() + i)) * 0.5f);
            
            float x = centerX + cosf(angle) * dist;
            float y = centerY + sinf(angle) * dist;
            
            DrawCircleV(raylib::Vector2(x, y), 5 * scale, 
                       LerpColor(VIOLET, PURPLE, (float)i / 8.0f));
        }
        
        // Magnetic vector
        float magAngle = atan2f(magValue.y, magValue.x);
        float magStrength = sqrtf(magValue.x * magValue.x + magValue.y * magValue.y);
        float vecLen = magStrength * radius * 0.8f;
        
        float endX = centerX + cosf(magAngle) * vecLen;
        float endY = centerY + sinf(magAngle) * vecLen;
        
        DrawLineEx(raylib::Vector2(centerX, centerY), raylib::Vector2(endX, endY), 
                   4 * scale, VIOLET);
        DrawCircleV(raylib::Vector2(endX, endY), 10 * scale, VIOLET);
        
        // Center
        DrawCircle(centerX, centerY, 15 * scale, raylib::Color(30, 30, 45, 255));
        DrawCircleLines(centerX, centerY, 15 * scale, VIOLET);
    }
    
    void DrawCompass(int centerX, int centerY, float radius) {
        float scale = GetScaleFactor();
        
        // Outer ring
        DrawCircleGradient(centerX, centerY, radius * 1.1f, 
                          Fade(LIME, 0.3f), Fade(LIME, 0.0f));
        DrawCircle(centerX, centerY, radius, raylib::Color(30, 30, 45, 255));
        DrawCircleLines(centerX, centerY, radius, LIME);
        DrawCircleLines(centerX, centerY, radius * 0.95f, DARKGREEN);
        
        // Cardinal directions
        const char* directions[] = {"N", "E", "S", "W"};
        raylib::Color dirColors[] = {RED, LIGHTGRAY, LIGHTGRAY, LIGHTGRAY};
        
        for (int i = 0; i < 4; i++) {
            float angle = (i * 90 - 90) * DEG2RAD;
            float x = centerX + cosf(angle) * radius * 0.8f;
            float y = centerY + sinf(angle) * radius * 0.8f;
            
            int textWidth = MeasureText(directions[i], 24 * scale);
            ::DrawText(directions[i], x - textWidth/2, y - 12 * scale, 24 * scale, dirColors[i]);
        }
        
        // Degree marks
        for (int i = 0; i < 36; i++) {
            float angle = (i * 10) * DEG2RAD;
            float r1 = radius * 0.9f;
            float r2 = (i % 3 == 0) ? radius * 0.85f : radius * 0.88f;
            
            float x1 = centerX + cosf(angle - PI/2) * r1;
            float y1 = centerY + sinf(angle - PI/2) * r1;
            float x2 = centerX + cosf(angle - PI/2) * r2;
            float y2 = centerY + sinf(angle - PI/2) * r2;
            
            DrawLineEx(raylib::Vector2(x1, y1), raylib::Vector2(x2, y2), 
                      2 * scale, DARKGREEN);
        }
        
        // Needle
        float needleAngle = (compassAngle - 90) * DEG2RAD;
        
        // North (red)
        raylib::Vector2 northTip(
            centerX + cosf(needleAngle) * radius * 0.7f,
            centerY + sinf(needleAngle) * radius * 0.7f
        );
        raylib::Vector2 base1(
            centerX + cosf(needleAngle + PI/2) * 8 * scale,
            centerY + sinf(needleAngle + PI/2) * 8 * scale
        );
        raylib::Vector2 base2(
            centerX + cosf(needleAngle - PI/2) * 8 * scale,
            centerY + sinf(needleAngle - PI/2) * 8 * scale
        );
        
        DrawTriangle(northTip, base1, base2, RED);
        DrawTriangleLines(northTip, base1, base2, DARKGRAY);
        
        // South (white)
        raylib::Vector2 southTip(
            centerX - cosf(needleAngle) * radius * 0.7f,
            centerY - sinf(needleAngle) * radius * 0.7f
        );
        
        DrawTriangle(southTip, base1, base2, LIGHTGRAY);
        DrawTriangleLines(southTip, base1, base2, DARKGRAY);
        
        // Center pivot
        DrawCircle(centerX, centerY, 12 * scale, YELLOW);
        DrawCircleLines(centerX, centerY, 12 * scale, GOLD);
        
        // Heading text
        ::DrawText(TextFormat("%.0f°", fmodf(compassAngle + 360.0f, 360.0f)), 
                 centerX - 30 * scale, centerY + radius + 30 * scale, 
                 24 * scale, YELLOW);
    }
    
    void DrawTabs() {
        float scale = GetScaleFactor();
        int tabWidth = GetGameWidth() / TAB_COUNT;
        int tabHeight = TAB_HEIGHT * scale;
        
        for (int i = 0; i < TAB_COUNT; i++) {
            int x = i * tabWidth;
            int y = TAB_HEIGHT * scale * 0.5f;
            
            raylib::Color tabColor = (i == currentTab) ? 
                raylib::Color(40, 40, 60, 255) : raylib::Color(25, 25, 40, 255);
            
            // Tab background
            DrawRectangle(x, y, tabWidth, tabHeight, tabColor);
            
            // Highlight if active
            if (i == currentTab) {
                DrawRectangle(x, y + tabHeight - 4 * scale, tabWidth, 4 * scale, SKYBLUE);
            }
            
            // Border
            DrawLine(x + tabWidth, y, x + tabWidth, y + tabHeight, 
                    raylib::Color(60, 60, 80, 255));
            
            // Tab text
            int textSize = 16 * scale;
            int textWidth = MeasureText(TAB_NAMES[i], textSize);
            ::DrawText(TAB_NAMES[i], 
                    x + (tabWidth - textWidth) / 2, 
                    y + (tabHeight - textSize) / 2,
                    textSize, 
                    (i == currentTab) ? WHITE : LIGHTGRAY);
        }
        
        // Handle tab clicks
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            raylib::Vector2 mousePos = GetMousePosition();
            if (mousePos.y >= TAB_HEIGHT * scale * 0.5f && 
                mousePos.y <= TAB_HEIGHT * scale * 1.5f) {
                int clickedTab = (int)(mousePos.x / tabWidth);
                if (clickedTab >= 0 && clickedTab < TAB_COUNT && clickedTab != currentTab) {
                    targetTab = clickedTab;
                    tabTransition = 1.0f;
                    currentTab = (SensorTab)clickedTab;
                }
            }
        }
    }
    
    void DrawHeader() {
        float scale = GetScaleFactor();
        
        // Header background
        DrawRectangleGradientV(0, 0, GetGameWidth(), TAB_HEIGHT * scale * 0.5f,
                               raylib::Color(20, 20, 35, 255),
                               raylib::Color(15, 15, 25, 255));
        
        // Title
        const char* title = "SENSOR DEMO";
        int titleSize = 28 * scale;
        int titleWidth = MeasureText(title, titleSize);
        ::DrawText(title, (GetGameWidth() - titleWidth) / 2, 10 * scale, titleSize, GOLD);
        
        // Subtitle
        const char* subtitle = "Android NDK Sensors Visualization";
        int subtitleSize = 12 * scale;
        int subtitleWidth = MeasureText(subtitle, subtitleSize);
        ::DrawText(subtitle, (GetGameWidth() - subtitleWidth) / 2, 
                45 * scale, subtitleSize, LIGHTGRAY);
    }
};

int main() {
    // Initialize window
#ifdef PLATFORM_ANDROID
    raylib::Window window(0, 0, "Sensor Demo");
#else
    raylib::Window window(800, 1200, "Sensor Demo");
#endif

    SetTargetFPS(60);
    
#ifdef PLATFORM_ANDROID
    std::cout << "Sensor Demo - Android Version" << std::endl;
#else
    std::cout << "Sensor Demo - Desktop Version (Simulated Sensors)" << std::endl;
#endif

    SensorDemoApp app;
    
    // Main loop
    while (!window.ShouldClose()) {
        app.Update();
        
        window.BeginDrawing();
        app.Draw();
    window.EndDrawing();
  }

    std::cout << "Sensor Demo finished!" << std::endl;
  return 0;
}
