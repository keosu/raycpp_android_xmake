#include <iostream>
#include <raylib-cpp/raylib-cpp.hpp>


int main() {
  // 初始化窗口
  const int screenWidth = 800;
  const int screenHeight = 600;

  raylib::Window window(screenWidth, screenHeight, "Raylib-CPP Demo");

  // 设置目标FPS
  SetTargetFPS(60);

  // 初始化一些变量用于演示
  raylib::Vector2 ballPosition = {screenWidth / 2.0f, screenHeight / 2.0f};
  raylib::Vector2 ballSpeed = {5.0f, 4.0f};
  float ballRadius = 20.0f;

  raylib::Color ballColor = MAROON;

  std::cout << "Raylib-CPP Demo Started!" << std::endl;
  std::cout << "Controls:" << std::endl;
  std::cout << "- Press SPACE to change ball color" << std::endl;
  std::cout << "- Press ESC to exit" << std::endl;

  // 主游戏循环
  while (!window.ShouldClose()) {
    // 更新
    // 移动球
    ballPosition.x += ballSpeed.x;
    ballPosition.y += ballSpeed.y;

    // 检查边界碰撞
    if ((ballPosition.x >= (screenWidth - ballRadius)) ||
        (ballPosition.x <= ballRadius)) {
      ballSpeed.x *= -1.0f;
    }
    if ((ballPosition.y >= (screenHeight - ballRadius)) ||
        (ballPosition.y <= ballRadius)) {
      ballSpeed.y *= -1.0f;
    }

    // 处理输入
    if (IsKeyPressed(KEY_SPACE)) {
      // 切换球的颜色
      static int colorIndex = 0;
      raylib::Color colors[] = {MAROON,     ORANGE, DARKGREEN, DARKBLUE,
                                DARKPURPLE, BROWN,  DARKGRAY,  RED};
      colorIndex = (colorIndex + 1) % (sizeof(colors) / sizeof(colors[0]));
      ballColor = colors[colorIndex];
    }

    // 绘制
    window.BeginDrawing();
    {
      window.ClearBackground(RAYWHITE);

      // 绘制标题
      DrawText("Raylib-CPP Demo", 10, 10, 20, DARKGRAY);

      // 绘制说明
      DrawText("Press SPACE to change color", 10, 40, 10, GRAY);
      DrawText("Press ESC to exit", 10, 55, 10, GRAY);

      // 绘制弹跳的球
      DrawCircleV(ballPosition, ballRadius, ballColor);

      // 绘制一些装饰性的图形
      DrawRectangle(10, screenHeight - 30, 200, 20, LIGHTGRAY);
      DrawText("Raylib-CPP is awesome!", 15, screenHeight - 25, 10, DARKGRAY);

      // 绘制FPS
      DrawFPS(screenWidth - 80, 10);
    }
    window.EndDrawing();
  }

  std::cout << "Demo finished!" << std::endl;
  return 0;
}
