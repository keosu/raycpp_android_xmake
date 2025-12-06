#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>
#include <deque>
#include <locale>
#include <raylib-cpp/raylib-cpp.hpp>
#include "strings.h"

// Windows console UTF-8 support - 声明所需函数避免包含整个windows.h
#ifdef _WIN32
extern "C" {
    __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int wCodePageID);
    __declspec(dllimport) int __stdcall SetConsoleCP(unsigned int wCodePageID);
}
#define CP_UTF8 65001
#endif

// Screen helper functions
inline int GetGameWidth() { return GetScreenWidth(); }
inline int GetGameHeight() { return GetScreenHeight(); }
inline bool IsPortrait() { return GetScreenHeight() > GetScreenWidth(); }

inline float GetScaleFactor() {
    // 横屏模式使用1400为基准，竖屏使用800为基准
    float baseWidth = IsPortrait() ? 800.0f : 1400.0f;
    float baseHeight = IsPortrait() ? 1200.0f : 900.0f;
    
    // 同时考虑宽度和高度，取较小的缩放比
    float scaleW = GetGameWidth() / baseWidth;
    float scaleH = GetGameHeight() / baseHeight;
    return (scaleW < scaleH) ? scaleW : scaleH;
}

// UI Constants
const int TAB_HEIGHT = 60;  // 减小标签栏高度
const int CARD_MARGIN = 20;
const int CARD_PADDING = 15;
const int GRAPH_HISTORY = 100;

// 辅助函数：从map获取标签名称
inline const char* GetTabNameCN(DemoTab tab) {
    auto it = Strings::TAB_NAMES_CN.find(tab);
    return it != Strings::TAB_NAMES_CN.end() ? it->second : "Unknown";
}

inline const char* GetTabNameEN(DemoTab tab) {
    auto it = Strings::TAB_NAMES_EN.find(tab);
    return it != Strings::TAB_NAMES_EN.end() ? it->second : "Unknown";
}

// Emoji versions (will be used if emoji font is available)
const char* TAB_EMOJIS[] = {
    "🔷",  // Shapes
    "📝",  // Text
    "🎨",  // Colors
    "⌨️",   // Input
    "✨",  // Animation
    "📦"   // 3D
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

// 使用 strings.h 中的定义

// Main raylib features demo app
class RaylibDemoApp {
private:
    DemoTab currentTab;
    
    // Demo data
    std::vector<TouchPoint> touchPoints;
    std::vector<Particle> particles;
    
    // Animation state
    float animTime;
    
    // UI state
    float tabTransition;
    int targetTab;
    bool showInfo;
    
    // Font system
    Font customFont;
    Font emojiFont;
    bool fontLoaded;
    bool emojiFontLoaded;
    
    // Icon textures
    Texture2D iconTextures[TAB_COUNT];
    bool iconsLoaded;
    
    // 3D visualization
    Camera3D camera;
    raylib::Vector3 cubeRotation;
    float cameraAngle;
    
    // Input tracking
    raylib::Vector2 mousePos;
    bool mousePressed;
    
    // Window size tracking for display changes
    int lastWidth;
    int lastHeight;
    int lastRenderWidth;
    int lastRenderHeight;
    
public:
    RaylibDemoApp() {
        currentTab = TAB_SHAPES;
        targetTab = TAB_SHAPES;
        tabTransition = 0.0f;
        showInfo = false;
        
        touchPoints.resize(10);
        
        cubeRotation = raylib::Vector3(0, 0, 0);
        cameraAngle = 0.0f;
        animTime = 0.0f;
        
        mousePos = raylib::Vector2(0, 0);
        mousePressed = false;
        
        // Initialize window size tracking
        lastWidth = GetScreenWidth();
        lastHeight = GetScreenHeight();
        lastRenderWidth = GetRenderWidth();
        lastRenderHeight = GetRenderHeight();
        
        fontLoaded = false;
        emojiFontLoaded = false;
        iconsLoaded = false;
        
        // Load Chinese font - ONLY from assets, no fallback to system fonts
        // Try multiple possible paths (different working directories)
        const char* fontPaths[] = {
            "assets/fonts/SourceHanSansSC-Regular.otf",        // 当前目录
            "assets/fonts/NotoSansSC-Regular.otf",
            "../../../assets/fonts/SourceHanSansSC-Regular.otf",  // 从 build/windows/x64/release
            "../../../assets/fonts/NotoSansSC-Regular.otf",
            "../../../../assets/fonts/SourceHanSansSC-Regular.otf",  // 其他可能路径
            "../assets/fonts/SourceHanSansSC-Regular.otf",
        };
        
        for (const char* path : fontPaths) {
            std::cout << "尝试加载字体: " << path << std::endl;
            
            if (FileExists(path)) {
                std::cout << "  文件存在，开始加载..." << std::endl;
                
                // Define codepoint ranges for Chinese characters
                // We need to load specific ranges to avoid memory issues
                int charsCount = 0;
                // 从strings.h收集所有中文字符，加上ASCII和特殊符号
                std::string allChars = 
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+-=[]{}|;:',.<>?/` °|"
                    + Strings::GetAllChineseChars();
                
                int *codepoints = LoadCodepoints(allChars.c_str(), &charsCount);
                
                std::cout << "  加载 " << charsCount << " 个常用字符..." << std::endl;
                
                customFont = LoadFontEx(path, 48, codepoints, charsCount);
                UnloadCodepoints(codepoints);
                
                if (customFont.texture.id != 0 && customFont.glyphCount > 100) {
                    fontLoaded = true;
                    SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);
                    std::cout << "✓ 成功加载字体: " << path << std::endl;
                    std::cout << "  字形数量: " << customFont.glyphCount << std::endl;
                    break;
                } else {
                    std::cout << "  字体加载失败（字形数: " << customFont.glyphCount << "）" << std::endl;
                }
            } else {
                std::cout << "  文件不存在，跳过..." << std::endl;
            }
        }
        
        // 如果字体加载失败，报错并退出
        if (!fontLoaded) {
            std::cerr << "\n❌ 错误: 无法加载中文字体！" << std::endl;
            std::cerr << "请确保以下任一字体文件存在于 assets/fonts/ 目录:" << std::endl;
            for (const char* path : fontPaths) {
                std::cerr << "  - " << path << std::endl;
            }
            std::cerr << "\n建议操作:" << std::endl;
            std::cerr << "  1. 运行 download_fonts.bat 下载字体" << std::endl;
            std::cerr << "  2. 或手动下载字体到 assets/fonts/ 目录" << std::endl;
            std::cerr << "  3. 重新编译: xmake build" << std::endl;
            std::cerr << "  4. 重新安装: xmake install -o build" << std::endl;
            std::cerr << "\n程序将退出..." << std::endl;
            
            // 等待用户查看错误信息
            #ifdef _WIN32
            system("pause");
            #endif
            
            exit(1);  // 退出程序
        }
        
        // Try to load emoji font (optional)
        const char* emojiFontPaths[] = {
            "assets/fonts/NotoColorEmoji.ttf",
            "assets/fonts/TwemojiColor.ttf",
            "../../../assets/fonts/NotoColorEmoji.ttf",
            "../assets/fonts/NotoColorEmoji.ttf"
        };
        
        for (const char* path : emojiFontPaths) {
            std::cout << "尝试加载 Emoji 字体: " << path << std::endl;
            
            if (FileExists(path)) {
                std::cout << "  文件存在，开始加载..." << std::endl;
                // Load all emoji glyphs
                emojiFont = LoadFontEx(path, 48, nullptr, 0);
                
                // 更严格的验证：检查texture和glyphCount
                if (emojiFont.texture.id != 0 && emojiFont.glyphCount > 10) {
                    emojiFontLoaded = true;
                    SetTextureFilter(emojiFont.texture, TEXTURE_FILTER_BILINEAR);
                    std::cout << "✓ 成功加载 Emoji 字体: " << path << std::endl;
                    std::cout << "  字形数量: " << emojiFont.glyphCount << std::endl;
                    break;
                } else {
                    std::cout << "  Emoji 字体加载失败（字形数: " << emojiFont.glyphCount << "）" << std::endl;
                }
            } else {
                std::cout << "  文件不存在，跳过..." << std::endl;
            }
        }
        
        // Emoji 是可选的，不强制要求
        if (!emojiFontLoaded) {
            std::cout << "\n⚠ 提示: Emoji 字体未加载，将使用文字替代" << std::endl;
            std::cout << "  如需 Emoji 支持，请下载 NotoColorEmoji.ttf 到 assets/fonts/" << std::endl;
        }
        
        // Setup 3D camera
        camera.position = raylib::Vector3(0.0f, 10.0f, 10.0f);
        camera.target = raylib::Vector3(0.0f, 0.0f, 0.0f);
        camera.up = raylib::Vector3(0.0f, 1.0f, 0.0f);
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;
        
        // Generate icon textures
        GenerateIconTextures();
    }
    
    // 生成图标纹理
    void GenerateIconTextures() {
        const int iconSize = 64;  // 图标尺寸
        
        for (int i = 0; i < TAB_COUNT; i++) {
            RenderTexture2D renderTexture = LoadRenderTexture(iconSize, iconSize);
            
            BeginTextureMode(renderTexture);
            ClearBackground(BLANK);
            
            // 根据不同标签绘制不同图标
            float centerX = iconSize / 2.0f;
            float centerY = iconSize / 2.0f;
            
            switch (i) {
                case TAB_SHAPES: {
                    // 几何图形：方形、圆形、三角形组合
                    DrawRectangle(12, 12, 18, 18, SKYBLUE);
                    DrawCircle(45, 20, 10, GREEN);
                    DrawTriangle(
                        raylib::Vector2(32, 35),
                        raylib::Vector2(24, 50),
                        raylib::Vector2(40, 50),
                        ORANGE
                    );
                    break;
                }
                case TAB_TEXT: {
                    // 文本：字母A
                    DrawText("A", 16, 10, 42, WHITE);
                    DrawRectangle(12, 52, 40, 4, WHITE);
                    break;
                }
                case TAB_COLORS: {
                    // 颜色：彩虹圆
                    for (int j = 0; j < 8; j++) {
                        float angle = j * 45.0f;
                        raylib::Color color = ColorFromHSV(angle, 0.8f, 1.0f);
                        DrawCircleSector(
                            raylib::Vector2(centerX, centerY),
                            24,
                            angle - 22.5f,
                            angle + 22.5f,
                            16,
                            color
                        );
                    }
                    DrawCircle(centerX, centerY, 10, raylib::Color(30, 30, 45, 255));
                    break;
                }
                case TAB_INPUT: {
                    // 输入：鼠标光标
                    DrawTriangle(
                        raylib::Vector2(20, 15),
                        raylib::Vector2(20, 45),
                        raylib::Vector2(40, 35),
                        WHITE
                    );
                    DrawTriangleLines(
                        raylib::Vector2(20, 15),
                        raylib::Vector2(20, 45),
                        raylib::Vector2(40, 35),
                        SKYBLUE
                    );
                    // 点击圆圈
                    DrawCircleLines(45, 20, 8, YELLOW);
                    break;
                }
                case TAB_ANIMATION: {
                    // 动画：旋转的圆点
                    for (int j = 0; j < 6; j++) {
                        float angle = (j * 60) * DEG2RAD;
                        float radius = 18;
                        float x = centerX + cosf(angle) * radius;
                        float y = centerY + sinf(angle) * radius;
                        float size = 3 + (j % 3) * 2;
                        DrawCircle(x, y, size, ColorFromHSV(j * 60.0f, 0.8f, 1.0f));
                    }
                    break;
                }
                case TAB_3D: {
                    // 3D：立方体线框
                    // 前面
                    DrawRectangleLines(18, 20, 20, 20, WHITE);
                    // 后面（偏移表示深度）
                    DrawRectangleLines(26, 12, 20, 20, LIGHTGRAY);
                    // 连接线
                    DrawLine(18, 20, 26, 12, SKYBLUE);
                    DrawLine(38, 20, 46, 12, SKYBLUE);
                    DrawLine(18, 40, 26, 32, SKYBLUE);
                    DrawLine(38, 40, 46, 32, SKYBLUE);
                    break;
                }
            }
            
            EndTextureMode();
            
            // 保存纹理
            iconTextures[i] = renderTexture.texture;
        }
        
        iconsLoaded = true;
        std::cout << "✓ 成功生成 " << TAB_COUNT << " 个图标纹理" << std::endl;
    }
    
    ~RaylibDemoApp() {
        if (fontLoaded) {
            UnloadFont(customFont);
        }
        if (emojiFontLoaded) {
            UnloadFont(emojiFont);
        }
        if (iconsLoaded) {
            for (int i = 0; i < TAB_COUNT; i++) {
                UnloadTexture(iconTextures[i]);
            }
        }
    }
    
    void Update() {
        // Detect window/display changes (e.g., moving to different monitor with different DPI)
        int currentWidth = GetScreenWidth();
        int currentHeight = GetScreenHeight();
        int currentRenderWidth = GetRenderWidth();
        int currentRenderHeight = GetRenderHeight();
        
        if (currentWidth != lastWidth || currentHeight != lastHeight || 
            currentRenderWidth != lastRenderWidth || currentRenderHeight != lastRenderHeight) {
            // Window or render size changed (e.g., moved to different monitor)
            lastWidth = currentWidth;
            lastHeight = currentHeight;
            lastRenderWidth = currentRenderWidth;
            lastRenderHeight = currentRenderHeight;
            
            std::cout << "窗口变化检测 - Screen: " << currentWidth << "x" << currentHeight 
                     << " | Render: " << currentRenderWidth << "x" << currentRenderHeight << std::endl;
        }
        
        animTime += GetFrameTime();
        UpdateInput();
        UpdateUI();
        UpdateParticles();
        Update3DVisualization();
    }
    
    void UpdateInput() {
        // Track mouse position and clicks
        mousePos = GetMousePosition();
        
        // Update touch points (for particle effects in animation tab)
        if (currentTab == TAB_ANIMATION || currentTab == TAB_INPUT) {
            // Handle mouse/touch input for particle effects
            int touchCount = GetTouchPointCount();
            if (touchCount > 0 || IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                for (int i = 0; i < std::max(1, touchCount); i++) {
                    raylib::Vector2 pos = touchCount > 0 ? GetTouchPosition(i) : mousePos;
                    
                    if (i < 10) {
                        if (!touchPoints[i].active) {
                            // New touch/click
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
                    }
                }
            } else {
                // Release all touches
                for (int i = 0; i < 10; i++) {
                    if (touchPoints[i].active) {
                        SpawnReleaseParticles(touchPoints[i].position, touchPoints[i].color);
                        touchPoints[i].active = false;
                    }
                    touchPoints[i].lifetime -= GetFrameTime();
                }
            }
        }
    }
    
    void UpdateUI() {
        // Tab switching
        if (tabTransition > 0) {
            tabTransition -= GetFrameTime() * 5.0f;
            if (tabTransition <= 0) {
                currentTab = (DemoTab)targetTab;
                tabTransition = 0;
            }
        }
        
        // Toggle info with key
        #ifndef PLATFORM_ANDROID
        if (IsKeyPressed(KEY_I)) {
            showInfo = !showInfo;
        }
        
        // 全屏切换 - F11 或 Alt+Enter
        if (IsKeyPressed(KEY_F11) || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER))) {
            int monitor = GetCurrentMonitor();
            if (IsWindowFullscreen()) {
                // 退出全屏
                SetWindowSize(1400, 900);
                ToggleFullscreen();
                std::cout << "退出全屏模式" << std::endl;
            } else {
                // 进入全屏
                SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
                ToggleFullscreen();
                std::cout << "进入全屏模式: " << GetMonitorWidth(monitor) << "x" << GetMonitorHeight(monitor) << std::endl;
            }
        }
        #else
        // On Android, double tap to toggle info
        static float lastTapTime = 0;
        static int tapCount = 0;
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            float currentTime = GetTime();
            if (currentTime - lastTapTime < 0.3f) {
                tapCount++;
                if (tapCount >= 1) {  // Double tap
                    showInfo = !showInfo;
                    tapCount = 0;
                }
            } else {
                tapCount = 0;
            }
            lastTapTime = currentTime;
        }
        #endif
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
        // Smoothly rotate 3D objects
        cubeRotation.x += GetFrameTime() * 20.0f;
        cubeRotation.y += GetFrameTime() * 30.0f;
        cubeRotation.z += GetFrameTime() * 10.0f;
        
        // Rotate camera around the scene
        cameraAngle += GetFrameTime() * 0.5f;
        camera.position.x = sinf(cameraAngle) * 10.0f;
        camera.position.z = cosf(cameraAngle) * 10.0f;
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
        
        // 绘制顺序：内容 -> 标签栏 -> 头部 -> 粒子效果（最顶层）
        DrawContent();
        DrawTabs();
        DrawHeader();
        DrawParticles();  // 粒子效果放在最顶层
    }
    
    void DrawParticles() {
        for (const auto& p : particles) {
            p.Draw();
        }
    }
    
    void DrawContent() {
        float scale = GetScaleFactor();
        int contentY = (int)(TAB_HEIGHT * 1.5f * scale);
        int contentHeight = GetGameHeight() - contentY;
        
        // 确保内容区域有效
        if (contentHeight < 100) {
            contentHeight = 100;
        }
        
        switch (currentTab) {
            case TAB_SHAPES:
                DrawShapesView(contentY, contentHeight);
                break;
            case TAB_TEXT:
                DrawTextView(contentY, contentHeight);
                break;
            case TAB_COLORS:
                DrawColorsView(contentY, contentHeight);
                break;
            case TAB_INPUT:
                DrawInputView(contentY, contentHeight);
                break;
            case TAB_ANIMATION:
                DrawAnimationView(contentY, contentHeight);
                break;
            case TAB_3D:
                Draw3DView(contentY, contentHeight);
                break;
            case TAB_COUNT:
                // Not a real tab
                break;
        }
    }
    
    void DrawShapesView(int y, int height) {
        float scale = GetScaleFactor();
        int width = GetGameWidth();
        
        // Title card
        int titleHeight = 85 * scale;
        DrawCard(CARD_MARGIN, y + CARD_MARGIN, width - CARD_MARGIN * 2, titleHeight);
        std::string title = Strings::SHAPES_TITLE;
        DrawTextCentered(title.c_str(), width/2, y + CARD_MARGIN + CARD_PADDING * 1.5, 24 * scale, SKYBLUE);
        DrawTextCentered(Strings::SHAPES_DESC, 
                 width/2, y + CARD_MARGIN + CARD_PADDING + 45 * scale, 
                 14 * scale, LIGHTGRAY);
        
        int infoHeight = 0;
        if (showInfo) {
            infoHeight = 200 * scale;
            DrawFeatureInfo(TAB_SHAPES, y + CARD_MARGIN + titleHeight + 10 * scale);
        }
        
        // 演示各种图形 - 自适应布局
        int demoY = y + CARD_MARGIN + titleHeight + (showInfo ? infoHeight + 10 : 10) * scale;
        int demoHeight = height - (demoY - y) - CARD_MARGIN;  // 修复：相对于y的高度
        
        // 确保有足够空间
        if (demoHeight < 150 * scale) {
            demoHeight = 150 * scale;
        }
        
        // 左右分栏或单栏布局（根据宽度自适应）
        bool useTwoColumns = width > 1000;
        float leftWidth = useTwoColumns ? (width/2 - CARD_MARGIN * 1.5f) : (width - CARD_MARGIN * 2);
        float rightWidth = useTwoColumns ? (width/2 - CARD_MARGIN * 1.5f) : (width - CARD_MARGIN * 2);
        float rightX = useTwoColumns ? (width/2 + CARD_MARGIN * 0.5f) : CARD_MARGIN;
        float rightY = useTwoColumns ? demoY : (demoY + demoHeight/2 + 10 * scale);
        
        // 左侧: 基本图形
        DrawCard(CARD_MARGIN, demoY, leftWidth, useTwoColumns ? (demoHeight - CARD_MARGIN) : (demoHeight/2 - 5 * scale));
        DrawTextWithFont(Strings::SHAPES_BASIC, CARD_MARGIN + CARD_PADDING, demoY + CARD_PADDING, 16 * scale, YELLOW);
        
        int shapeX = CARD_MARGIN + leftWidth/2;
        int shapeY = demoY + 60 * scale;
        int maxSpacing = 100 * scale;
        int calcSpacing = (useTwoColumns ? (demoHeight - CARD_MARGIN) : (demoHeight/2 - 5 * scale)) / 4;
        int spacing = (maxSpacing < calcSpacing) ? maxSpacing : calcSpacing;
        
        // 圆形
        DrawCircle(shapeX, shapeY, 40 * scale, RED);
        DrawText(Strings::SHAPES_CIRCLE, shapeX - 25 * scale, shapeY + 55 * scale, 14 * scale, WHITE);
        
        // 矩形
        DrawRectangle(shapeX - 35 * scale, shapeY + spacing - 40 * scale, 70 * scale, 80 * scale, GREEN);
        DrawText(Strings::SHAPES_RECTANGLE, shapeX - 42 * scale, shapeY + spacing + 55 * scale, 14 * scale, WHITE);
        
        // 三角形
        DrawTriangle(
            raylib::Vector2(shapeX, shapeY + spacing * 2 - 40 * scale),
            raylib::Vector2(shapeX - 40 * scale, shapeY + spacing * 2 + 40 * scale),
            raylib::Vector2(shapeX + 40 * scale, shapeY + spacing * 2 + 40 * scale),
            BLUE
        );
        DrawText(Strings::SHAPES_TRIANGLE, shapeX - 35 * scale, shapeY + spacing * 2 + 55 * scale, 14 * scale, WHITE);
        
        // 右侧: 高级图形
        float cardHeight = useTwoColumns ? (demoHeight - CARD_MARGIN) : (demoHeight/2 - 5 * scale);
        DrawCard(rightX, rightY, rightWidth, cardHeight);
        DrawTextWithFont(Strings::SHAPES_ADVANCED, rightX + CARD_PADDING, rightY + CARD_PADDING, 16 * scale, YELLOW);
        
        int rightShapeX = rightX + rightWidth/2;
        int rightShapeY = rightY + 60 * scale;
        
        // 渐变圆
        DrawCircleGradient(rightShapeX, rightShapeY, 35 * scale, ORANGE, RED);
        DrawText(Strings::SHAPES_GRADIENT, rightShapeX - 35 * scale, rightShapeY + 50 * scale, 13 * scale, WHITE);
        
        // 抗锯齿圆环
        DrawRing(raylib::Vector2(rightShapeX, rightShapeY + spacing), 25 * scale, 40 * scale, 0, 360, 72, PURPLE);
        DrawText(Strings::SHAPES_RING, rightShapeX - 38 * scale, rightShapeY + spacing + 50 * scale, 13 * scale, WHITE);
        
        // 线条（如果空间足够）
        if (spacing * 3 < cardHeight) {
            float time = animTime;
            for (int i = 0; i < 8; i++) {
                float angle = (i * 45 + time * 50) * DEG2RAD;
                DrawLineEx(
                    raylib::Vector2(rightShapeX, rightShapeY + spacing * 2),
                    raylib::Vector2(rightShapeX + cosf(angle) * 45 * scale, rightShapeY + spacing * 2 + sinf(angle) * 45 * scale),
                    2.5f * scale,
                    ColorFromHSV(i * 45.0f, 0.8f, 1.0f)
                );
            }
            DrawText(Strings::SHAPES_LINES, rightShapeX - 25 * scale, rightShapeY + spacing * 2 + 50 * scale, 13 * scale, WHITE);
        }
    }
    
    void DrawTextView(int y, int height) {
        float scale = GetScaleFactor();
        int width = GetGameWidth();
        
        // Title card
        int titleHeight = 85 * scale;
        DrawCard(CARD_MARGIN, y + CARD_MARGIN, width - CARD_MARGIN * 2, titleHeight);
        DrawTextCentered(Strings::TEXT_TITLE, width/2, y + CARD_MARGIN + CARD_PADDING * 1.5, 24 * scale, ORANGE);
        DrawTextCentered(Strings::TEXT_DESC, 
                 width/2, y + CARD_MARGIN + CARD_PADDING + 45 * scale, 
                 14 * scale, LIGHTGRAY);
        
        int infoHeight = 0;
        if (showInfo) {
            infoHeight = 200 * scale;
            DrawFeatureInfo(TAB_TEXT, y + CARD_MARGIN + titleHeight + 10 * scale);
        }
        
        int demoY = y + CARD_MARGIN + titleHeight + (showInfo ? infoHeight + 10 : 10) * scale;
        int demoHeight = height - (demoY - y) - CARD_MARGIN;
        int startY = demoY + 40 * scale;
        
        // 演示不同大小和颜色的文本
        DrawCard(CARD_MARGIN, demoY, width - CARD_MARGIN * 2, demoHeight);
        
        // 默认字体
        DrawText("Default Font - Raylib 默认字体", CARD_MARGIN + CARD_PADDING, startY, 20 * scale, WHITE);
        DrawText("Default Font - Small Size 小号", CARD_MARGIN + CARD_PADDING, startY + 35 * scale, 14 * scale, LIGHTGRAY);
        
        // 自定义字体 - 不同大小
        startY += 80 * scale;
        DrawTextWithFont("自定义字体 Custom Font - 大号 Large", CARD_MARGIN + CARD_PADDING, startY, 32 * scale, GOLD);
        
        startY += 50 * scale;
        DrawTextWithFont("自定义字体 中号 Medium", CARD_MARGIN + CARD_PADDING, startY, 24 * scale, SKYBLUE);
        
        startY += 40 * scale;
        DrawTextWithFont("自定义字体 小号 Small", CARD_MARGIN + CARD_PADDING, startY, 18 * scale, LIME);
        
        // Unicode 支持演示
        startY += 50 * scale;
        DrawTextWithFont("Unicode: 中文 日本語 한국어 Русский", CARD_MARGIN + CARD_PADDING, startY, 20 * scale, VIOLET);
        
        // 彩色文本组合
        startY += 50 * scale;
        int textX = CARD_MARGIN + CARD_PADDING;
        DrawTextWithFont("彩", textX, startY, 36 * scale, RED);
        textX += 45 * scale;
        DrawTextWithFont("色", textX, startY, 36 * scale, ORANGE);
        textX += 45 * scale;
        DrawTextWithFont("文", textX, startY, 36 * scale, YELLOW);
        textX += 45 * scale;
        DrawTextWithFont("字", textX, startY, 36 * scale, GREEN);
        textX += 45 * scale;
        DrawTextWithFont("渲", textX, startY, 36 * scale, BLUE);
        textX += 45 * scale;
        DrawTextWithFont("染", textX, startY, 36 * scale, PURPLE);
        
        textX += 60 * scale;
        DrawText("Colored", textX, startY, 36 * scale, SKYBLUE);
        textX += 150 * scale;
        DrawText("Text", textX, startY, 36 * scale, PINK);
        
        // 动画文本
        startY += 80 * scale;
        float wave = sinf(animTime * 2.0f) * 10;
        raylib::Color animColor = ColorFromHSV(fmodf(animTime * 50, 360), 0.8f, 1.0f);
        DrawTextWithFont("动画文本 Animated Text", CARD_MARGIN + CARD_PADDING, startY + wave, 26 * scale, animColor);
    }
    
    void DrawColorsView(int y, int height) {
        float scale = GetScaleFactor();
        int width = GetGameWidth();
        
        // Title card
        int titleHeight = 85 * scale;
        DrawCard(CARD_MARGIN, y + CARD_MARGIN, width - CARD_MARGIN * 2, titleHeight);
        DrawTextCentered(Strings::COLORS_TITLE, width/2, y + CARD_MARGIN + CARD_PADDING * 1.5, 24 * scale, VIOLET);
        DrawTextCentered(Strings::COLORS_DESC, 
                 width/2, y + CARD_MARGIN + CARD_PADDING + 45 * scale, 
                 14 * scale, LIGHTGRAY);
        
        int infoHeight = 0;
        if (showInfo) {
            infoHeight = 200 * scale;
            DrawFeatureInfo(TAB_COLORS, y + CARD_MARGIN + titleHeight + 10 * scale);
        }
        
        int demoY = y + CARD_MARGIN + titleHeight + (showInfo ? infoHeight + 10 : 10) * scale;
        int demoHeight = height - (demoY - y) - CARD_MARGIN;
        DrawCard(CARD_MARGIN, demoY, width - CARD_MARGIN * 2, demoHeight);
        
        int startX = CARD_MARGIN + CARD_PADDING + 20 * scale;
        int startY = demoY + 40 * scale;
        int boxSize = 80 * scale;
        int spacing = 100 * scale;
        
        // 基本颜色
        DrawTextWithFont("基本颜色 Basic Colors:", startX, startY, 20 * scale, YELLOW);
        startY += 40 * scale;
        
        raylib::Color basicColors[] = {RED, ORANGE, YELLOW, GREEN, SKYBLUE, BLUE, PURPLE, PINK};
        const char* colorNames[] = {"红", "橙", "黄", "绿", "青", "蓝", "紫", "粉"};
        for (int i = 0; i < 8; i++) {
            int x = startX + (i % 8) * spacing;
            DrawRectangle(x, startY, boxSize, boxSize, basicColors[i]);
            DrawText(colorNames[i], x + 25 * scale, startY + boxSize + 10 * scale, 16 * scale, WHITE);
        }
        
        // HSV 彩虹
        startY += 140 * scale;
        DrawTextWithFont("HSV 彩虹 Rainbow:", startX, startY, 20 * scale, YELLOW);
        startY += 40 * scale;
        
        for (int i = 0; i < width - CARD_MARGIN * 2 - CARD_PADDING * 2; i++) {
            float hue = (i * 360.0f) / (width - CARD_MARGIN * 2 - CARD_PADDING * 2);
            raylib::Color col = ColorFromHSV(hue, 1.0f, 1.0f);
            DrawRectangle(startX + i, startY, 1, 60 * scale, col);
        }
        
        // 渐变效果
        startY += 100 * scale;
        DrawTextWithFont("渐变 Gradients:", startX, startY, 20 * scale, YELLOW);
        startY += 40 * scale;
        
        int gradWidth = (width - CARD_MARGIN * 2 - CARD_PADDING * 2 - 60 * scale) / 3;
        
        // 垂直渐变
        DrawRectangleGradientV(startX, startY, gradWidth, 80 * scale, RED, BLUE);
        DrawText("Vertical", startX + 10 * scale, startY + 90 * scale, 14 * scale, WHITE);
        
        // 水平渐变
        DrawRectangleGradientH(startX + gradWidth + 30 * scale, startY, gradWidth, 80 * scale, GREEN, YELLOW);
        DrawText("Horizontal", startX + gradWidth + 40 * scale, startY + 90 * scale, 14 * scale, WHITE);
        
        // 径向渐变
        DrawRectangle(startX + gradWidth * 2 + 60 * scale, startY, gradWidth, 80 * scale, raylib::Color(20, 20, 30, 255));
        DrawCircleGradient(startX + gradWidth * 2 + 60 * scale + gradWidth/2, startY + 40 * scale, 50 * scale, ORANGE, Fade(ORANGE, 0));
        DrawText("Radial", startX + gradWidth * 2 + 70 * scale, startY + 90 * scale, 14 * scale, WHITE);
        
        // 透明度演示
        startY += 140 * scale;
        DrawTextWithFont("透明度 Transparency:", startX, startY, 20 * scale, YELLOW);
        startY += 40 * scale;
        
        for (int i = 0; i < 5; i++) {
            float alpha = (i + 1) / 5.0f;
            DrawRectangle(startX + i * (boxSize + 20 * scale), startY, boxSize, boxSize, Fade(PURPLE, alpha));
            DrawText(TextFormat("%.0f%%", alpha * 100), startX + i * (boxSize + 20 * scale) + 15 * scale, 
                     startY + boxSize + 10 * scale, 14 * scale, WHITE);
        }
    }
    
    void DrawInputView(int y, int height) {
        float scale = GetScaleFactor();
        int width = GetGameWidth();
        
        // Title card
        int titleHeight = 85 * scale;
        DrawCard(CARD_MARGIN, y + CARD_MARGIN, width - CARD_MARGIN * 2, titleHeight);
        DrawTextCentered(Strings::INPUT_TITLE, width/2, y + CARD_MARGIN + CARD_PADDING * 1.5, 24 * scale, GOLD);
        DrawTextCentered(Strings::INPUT_DESC, 
                 width/2, y + CARD_MARGIN + CARD_PADDING + 45 * scale, 
                 14 * scale, LIGHTGRAY);
        
        int infoHeight = 0;
        if (showInfo) {
            infoHeight = 200 * scale;
            DrawFeatureInfo(TAB_INPUT, y + CARD_MARGIN + titleHeight + 10 * scale);
        }
        
        int demoY = y + CARD_MARGIN + titleHeight + (showInfo ? infoHeight + 10 : 10) * scale;
        int demoHeight = height - (demoY - y) - CARD_MARGIN;
        DrawCard(CARD_MARGIN, demoY, width - CARD_MARGIN * 2, demoHeight);
        
        int startX = CARD_MARGIN + CARD_PADDING + 20 * scale;
        int startY = demoY + 40 * scale;
        
        // 鼠标信息
        DrawTextWithFont("鼠标信息 Mouse Info:", startX, startY, 22 * scale, SKYBLUE);
        startY += 40 * scale;
        
        DrawTextWithFont(TextFormat("位置 Position: (%.0f, %.0f)", mousePos.x, mousePos.y), 
                        startX + 20 * scale, startY, 18 * scale, WHITE);
        startY += 35 * scale;
        
        const char* mouseState = IsMouseButtonDown(MOUSE_LEFT_BUTTON) ? "按下 Down" : "释放 Up";
        raylib::Color mouseColor = IsMouseButtonDown(MOUSE_LEFT_BUTTON) ? GREEN : RED;
        DrawTextWithFont(TextFormat("左键 Left Button: %s", mouseState), 
                        startX + 20 * scale, startY, 18 * scale, mouseColor);
        startY += 35 * scale;
        
        float wheelMove = GetMouseWheelMove();
        DrawTextWithFont(TextFormat("滚轮 Wheel: %.1f", wheelMove), 
                        startX + 20 * scale, startY, 18 * scale, WHITE);
        
        // 键盘信息
        startY += 60 * scale;
        DrawTextWithFont("键盘信息 Keyboard Info:", startX, startY, 22 * scale, SKYBLUE);
        startY += 40 * scale;
        
        DrawTextWithFont("按下任意键测试 Press any key to test", 
                        startX + 20 * scale, startY, 18 * scale, LIGHTGRAY);
        startY += 35 * scale;
        
        int key = GetKeyPressed();
        if (key != 0) {
            DrawTextWithFont(TextFormat("最后按键 Last Key: %c (%d)", (char)key, key), 
                            startX + 20 * scale, startY, 18 * scale, YELLOW);
        }
        
        // 交互区域
        startY += 60 * scale;
        DrawTextWithFont("交互区域 Interactive Area:", startX, startY, 22 * scale, SKYBLUE);
        startY += 40 * scale;
        DrawTextWithFont("点击鼠标在此区域绘制 Click to draw", 
                        startX + 20 * scale, startY, 16 * scale, LIGHTGRAY);
        startY += 35 * scale;
        
        int interactY = startY;
        int interactHeight = height - (startY - demoY) - 40 * scale;
        DrawRectangle(startX, interactY, width - CARD_MARGIN * 2 - CARD_PADDING * 2 - 40 * scale, 
                     interactHeight, raylib::Color(30, 30, 45, 255));
        DrawRectangleLines(startX, interactY, width - CARD_MARGIN * 2 - CARD_PADDING * 2 - 40 * scale, 
                          interactHeight, SKYBLUE);
        
        // 绘制鼠标轨迹
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            DrawCircleV(mousePos, 10 * scale, RED);
            if (particles.size() < 1000) {
                SpawnTrailParticles(mousePos, GOLD);
            }
        }
        
        // 绘制活跃的触摸点/粒子
        for (const auto& tp : touchPoints) {
            if (tp.active) {
                DrawCircleGradient(tp.position.x, tp.position.y, 40 * scale, 
                                   Fade(tp.color, 0.3f), Fade(tp.color, 0.0f));
                DrawCircleV(tp.position, 20 * scale, tp.color);
            }
        }
    }
    
    void DrawAnimationView(int y, int height) {
        float scale = GetScaleFactor();
        int width = GetGameWidth();
        
        // Title card
        int titleHeight = 85 * scale;
        DrawCard(CARD_MARGIN, y + CARD_MARGIN, width - CARD_MARGIN * 2, titleHeight);
        DrawTextCentered(Strings::ANIM_TITLE, width/2, y + CARD_MARGIN + CARD_PADDING * 1.5, 24 * scale, LIME);
        DrawTextCentered(Strings::ANIM_DESC, 
                 width/2, y + CARD_MARGIN + CARD_PADDING + 45 * scale, 
                 14 * scale, LIGHTGRAY);
        
        int infoHeight = 0;
        if (showInfo) {
            infoHeight = 200 * scale;
            DrawFeatureInfo(TAB_ANIMATION, y + CARD_MARGIN + titleHeight + 10 * scale);
        }
        
        int demoY = y + CARD_MARGIN + titleHeight + (showInfo ? infoHeight + 10 : 10) * scale;
        int demoHeight = height - (demoY - y) - CARD_MARGIN;
        DrawCard(CARD_MARGIN, demoY, width - CARD_MARGIN * 2, demoHeight);
        
        int startX = CARD_MARGIN + CARD_PADDING + 20 * scale;
        int startY = demoY + 40 * scale;
        
        // 粒子统计
        DrawTextWithFont(TextFormat("粒子数量 Particles: %d", (int)particles.size()), 
                        startX, startY, 20 * scale, SKYBLUE);
        DrawTextWithFont("点击鼠标创建粒子 Click to create particles", 
                        startX + 300 * scale, startY, 18 * scale, LIGHTGRAY);
        
        // 旋转动画圆
        startY += 80 * scale;
        for (int i = 0; i < 8; i++) {
            float angle = (i * 45 + animTime * 90) * DEG2RAD;
            float radius = 100 * scale + sinf(animTime * 2 + i) * 20 * scale;
            float x = width/2 + cosf(angle) * radius;
            float y = startY + 120 * scale + sinf(angle) * radius;
            raylib::Color col = ColorFromHSV(fmodf(animTime * 50 + i * 45, 360), 0.8f, 1.0f);
            DrawCircle(x, y, 15 * scale, col);
        }
        
        // 波浪动画
        startY += 300 * scale;
        DrawTextWithFont("波浪动画 Wave Animation", startX, startY - 40 * scale, 18 * scale, YELLOW);
        
        for (int i = 0; i < width - CARD_MARGIN * 2 - CARD_PADDING * 2 - 40 * scale; i += 10) {
            float wave1 = sinf((i + animTime * 100) * 0.02f) * 30 * scale;
            float wave2 = sinf((i + animTime * 80) * 0.03f + 1) * 20 * scale;
            
            raylib::Color col1 = ColorFromHSV(fmodf(i * 0.2f + animTime * 30, 360), 0.8f, 1.0f);
            raylib::Color col2 = ColorFromHSV(fmodf(i * 0.2f + animTime * 30 + 180, 360), 0.8f, 1.0f);
            
            DrawCircle(startX + i, startY + wave1, 5 * scale, col1);
            DrawCircle(startX + i, startY + 60 * scale + wave2, 5 * scale, col2);
        }
    }
    
    void Draw3DView(int y, int height) {
        float scale = GetScaleFactor();
        int width = GetGameWidth();
        
        // Title card
        int titleHeight = 85 * scale;
        DrawCard(CARD_MARGIN, y + CARD_MARGIN, width - CARD_MARGIN * 2, titleHeight);
        DrawTextCentered(Strings::TD_TITLE, width/2, y + CARD_MARGIN + CARD_PADDING * 1.5, 24 * scale, PURPLE);
        DrawTextCentered(Strings::TD_DESC, 
                 width/2, y + CARD_MARGIN + CARD_PADDING + 45 * scale, 
                 14 * scale, LIGHTGRAY);
        
        int infoHeight = 0;
        if (showInfo) {
            infoHeight = 200 * scale;
            DrawFeatureInfo(TAB_3D, y + CARD_MARGIN + titleHeight + 10 * scale);
        }
        
        int demoY = y + CARD_MARGIN + titleHeight + (showInfo ? infoHeight + 10 : 10) * scale;
        int demoHeight = height - (demoY - y) - CARD_MARGIN;
        
        // 3D 视口
        DrawCard(CARD_MARGIN, demoY, width - CARD_MARGIN * 2, demoHeight);
        
        // 3D 渲染
        BeginMode3D(camera);
        
        // 绘制网格
        DrawGrid(10, 1.0f);
        
        // 绘制旋转的立方体
        rlPushMatrix();
        rlRotatef(cubeRotation.x, 1, 0, 0);
        rlRotatef(cubeRotation.y, 0, 1, 0);
        rlRotatef(cubeRotation.z, 0, 0, 1);
        
        DrawCube(raylib::Vector3(0, 2, 0), 3.0f, 3.0f, 3.0f, SKYBLUE);
        DrawCubeWires(raylib::Vector3(0, 2, 0), 3.0f, 3.0f, 3.0f, DARKBLUE);
        
        rlPopMatrix();
        
        // 绘制几个彩色球体
        DrawSphere(raylib::Vector3(-4, 1.5f, 0), 1.0f, RED);
        DrawSphere(raylib::Vector3(4, 1.5f, 0), 1.0f, GREEN);
        DrawSphere(raylib::Vector3(0, 1.5f, -4), 1.0f, YELLOW);
        DrawSphere(raylib::Vector3(0, 1.5f, 4), 1.0f, PURPLE);
        
        // 绘制坐标轴
        DrawLine3D(raylib::Vector3(0, 0, 0), raylib::Vector3(5, 0, 0), RED);
        DrawLine3D(raylib::Vector3(0, 0, 0), raylib::Vector3(0, 5, 0), GREEN);
        DrawLine3D(raylib::Vector3(0, 0, 0), raylib::Vector3(0, 0, 5), BLUE);
        
        EndMode3D();
        
        // 3D 信息
        int infoX = CARD_MARGIN + CARD_PADDING + 20 * scale;
        int infoY = demoY + CARD_PADDING + 10 * scale;
        DrawTextWithFont(TextFormat("%s (%.1f, %.1f, %.1f)", Strings::TD3_CAMERA,
                           camera.position.x, camera.position.y, camera.position.z),
                infoX, infoY, 15 * scale, WHITE);
        DrawTextWithFont(TextFormat("%s (%.0f°, %.0f°, %.0f°)", Strings::TD3_ROTATION,
                           cubeRotation.x, cubeRotation.y, cubeRotation.z),
                infoX, infoY + 23 * scale, 15 * scale, WHITE);
    }
    
    void DrawTextCentered(const char* text, int centerX, int y, int fontSize, raylib::Color color) {
        if (fontLoaded && fontSize >= 14) {
            // Use custom font for text
            float spacing = fontSize / 32.0f;
            Vector2 textSize = MeasureTextEx(customFont, text, fontSize, spacing);
            ::DrawTextEx(customFont, text, raylib::Vector2(centerX - textSize.x / 2, y), fontSize, spacing, color);
        } else {
            // Use default font for small text
            int textWidth = MeasureText(text, fontSize);
            ::DrawText(text, centerX - textWidth / 2, y, fontSize, color);
        }
    }
    
    void DrawTextWithFont(const char* text, int x, int y, int fontSize, raylib::Color color) {
        if (fontLoaded && fontSize >= 14) {
            float spacing = fontSize / 32.0f;
            ::DrawTextEx(customFont, text, raylib::Vector2(x, y), fontSize, spacing, color);
        } else {
            ::DrawText(text, x, y, fontSize, color);
        }
    }
    
    void DrawTextWithEmoji(const char* text, int x, int y, int fontSize, raylib::Color color) {
        // Try to detect and render emojis separately
        if (emojiFontLoaded) {
            // Simple emoji detection (Unicode range check would be better)
            bool hasEmoji = false;
            for (const char* p = text; *p; p++) {
                if ((unsigned char)*p > 0xF0) {  // Potential emoji start
                    hasEmoji = true;
                    break;
                }
            }
            
            if (hasEmoji) {
                float spacing = fontSize / 32.0f;
                ::DrawTextEx(emojiFont, text, raylib::Vector2(x, y), fontSize, spacing, color);
                return;
            }
        }
        
        // Fall back to regular text
        DrawTextWithFont(text, x, y, fontSize, color);
    }
    
    void DrawFeatureInfo(DemoTab tab, int y) {
        float scale = GetScaleFactor();
        int width = GetGameWidth() - CARD_MARGIN * 2;
        int height = 180 * scale;  // 减小高度
        
        DrawCard(CARD_MARGIN, y, width, height);
        
        auto it = Strings::FEATURES.find(tab);
        if (it == Strings::FEATURES.end()) return;
        const Strings::FeatureText& info = it->second;
        int centerX = GetGameWidth() / 2;
        int textY = y + CARD_PADDING * 1.5;
        
        // Chinese name and description
        DrawTextCentered(info.nameCN, centerX, textY, 20 * scale, YELLOW);
        textY += 32 * scale;
        
        // Chinese description
        DrawTextCentered(info.descCN, centerX, textY, 14 * scale, LIGHTGRAY);
        textY += 25 * scale;
        
        // English description
        DrawTextCentered(info.descEN, centerX, textY, 12 * scale, GRAY);
        textY += 32 * scale;
        
        // Usage label
        DrawTextCentered(Strings::INFO_LABEL, centerX, textY, 14 * scale, SKYBLUE);
        textY += 25 * scale;
        
        // Usage examples
        DrawTextCentered(info.usageCN, centerX, textY, 13 * scale, WHITE);
        textY += 22 * scale;
        DrawTextCentered(info.usageEN, centerX, textY, 11 * scale, LIGHTGRAY);
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
            
            // Tab图标 + 中文标签
            raylib::Color activeColor = WHITE;
            raylib::Color inactiveColor = LIGHTGRAY;
            raylib::Color textColor = (i == currentTab) ? activeColor : inactiveColor;
            
            if (iconsLoaded) {
                // 使用生成的图标纹理
                int iconSize = 32 * scale;
                int cnSize = 13 * scale;
                
                // 获取中文标签文本
                const char* tabName = GetTabNameCN((DemoTab)i);
                
                // 测量中文标签大小
                float cnSpacing = cnSize / 32.0f;
                Vector2 cnTextSize = MeasureTextEx(customFont, tabName, cnSize, cnSpacing);
                
                // 计算总高度和起始位置
                int totalHeight = iconSize + cnTextSize.y + 3 * scale;
                int startY = y + (tabHeight - totalHeight) / 2;
                
                // 绘制图标纹理（居中）
                int iconX = x + (tabWidth - iconSize) / 2;
                Rectangle source = {0, 0, (float)iconTextures[i].width, -(float)iconTextures[i].height};
                Rectangle dest = {(float)iconX, (float)startY, (float)iconSize, (float)iconSize};
                DrawTexturePro(iconTextures[i], source, dest, raylib::Vector2(0, 0), 0, textColor);
                
                // 绘制中文标签（居中）
                int cnX = x + (tabWidth - cnTextSize.x) / 2;
                ::DrawTextEx(customFont, tabName, 
                           raylib::Vector2(cnX, startY + iconSize + 3 * scale), 
                           cnSize, cnSpacing, textColor);
            } else {
                // 备用：使用文字标签
                const char* tabName = GetTabNameCN((DemoTab)i);
                int cnSize = 16 * scale;
                float cnSpacing = cnSize / 32.0f;
                Vector2 cnTextSize = MeasureTextEx(customFont, tabName, cnSize, cnSpacing);
                int cnX = x + (tabWidth - cnTextSize.x) / 2;
                int cnY = y + (tabHeight - cnTextSize.y) / 2;
                ::DrawTextEx(customFont, tabName, 
                           raylib::Vector2(cnX, cnY), 
                           cnSize, cnSpacing, textColor);
            }
        }
        
        // Handle tab clicks - 修复点击区域判断
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            raylib::Vector2 mousePos = GetMousePosition();
            // 标签栏的实际Y范围：从 TAB_HEIGHT * scale * 0.5f 到 TAB_HEIGHT * scale * 1.5f
            float tabStartY = TAB_HEIGHT * scale * 0.5f;
            float tabEndY = TAB_HEIGHT * scale * 1.5f;
            
            if (mousePos.y >= tabStartY && mousePos.y <= tabEndY) {
                // 确保点击在有效标签范围内
                int clickedTab = (int)(mousePos.x / tabWidth);
                if (clickedTab >= 0 && clickedTab < TAB_COUNT && clickedTab != currentTab) {
                    targetTab = clickedTab;
                    tabTransition = 1.0f;
                    currentTab = (DemoTab)clickedTab;
                    std::cout << "切换到标签: " << GetTabNameCN(currentTab) << std::endl;
                }
            }
        }
    }
    
    void DrawHeader() {
        float scale = GetScaleFactor();
        int centerX = GetGameWidth() / 2;
        
        // Header background
        DrawRectangleGradientV(0, 0, GetGameWidth(), TAB_HEIGHT * scale * 0.5f,
                               raylib::Color(20, 20, 35, 255),
                               raylib::Color(15, 15, 25, 255));
        
        // Title
        DrawTextCentered(Strings::APP_TITLE, centerX, 8 * scale, 22 * scale, GOLD);
        
        // 快捷键提示（右上角）
        #ifndef PLATFORM_ANDROID
        if (!showInfo) {
            int hintX = GetGameWidth() - 10 * scale;
            ::DrawText("Press I - Info", hintX - 95, 8 * scale, 10 * scale, GRAY);
            ::DrawText("F11 - Fullscreen", hintX - 110, 20 * scale, 10 * scale, GRAY);
        }
        #else
        if (!showInfo) {
            DrawTextWithFont(Strings::INFO_HINT_MOBILE, 10 * scale, 10 * scale, 10 * scale, GRAY);
        }
        #endif
    }
};

int main() {
    // 设置Windows控制台为UTF-8编码，解决中文乱码问题
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    #endif
    
    std::setlocale(LC_ALL, "");
    
    // Initialize window - 横屏布局，支持窗口大小调整和最大化
#ifdef PLATFORM_ANDROID
    raylib::Window window(0, 0, "Raylib 特性演示 Features Demo");
#else
    // 设置窗口可调整大小、最大化和高DPI支持
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    raylib::Window window(1400, 900, "Raylib 特性演示 Features Demo");
    SetWindowMinSize(1000, 700);  // 设置最小窗口大小
    
    // 注册全屏快捷键提示
    std::cout << "提示：按 F11 或 Alt+Enter 切换全屏" << std::endl;
#endif

    SetTargetFPS(60);
    
#ifdef PLATFORM_ANDROID
    std::cout << "Raylib 特性演示 - Android 版本" << std::endl;
#else
    std::cout << "Raylib 特性演示 - 桌面版本" << std::endl;
    std::cout << "支持窗口缩放和最大化" << std::endl;
#endif

    RaylibDemoApp app;
    
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
