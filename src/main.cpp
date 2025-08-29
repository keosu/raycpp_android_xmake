#include <iostream>
#include <raylib-cpp/raylib-cpp.hpp>

int main() {
  // Initialize window
  const int screenWidth = 800;
  const int screenHeight = 600;

  raylib::Window window(screenWidth, screenHeight, "Raylib-CPP Demo");

  // Set target FPS
  SetTargetFPS(60);

#ifdef PLATFORM_ANDROID
  // Android-specific initialization code
  std::cout << "Running on Android platform" << std::endl;
#else
  // Non-Android platform initialization code
  std::cout << "Running on non-Android platform" << std::endl;
  std::cout << "Controls:" << std::endl;
  std::cout << "- Press SPACE to change ball color" << std::endl;
  std::cout << "- Press ESC to exit" << std::endl;
#endif

  // Initialize variables for demonstration
  raylib::Vector2 ballPosition = {screenWidth / 2.0f, screenHeight / 2.0f};
  raylib::Vector2 ballSpeed = {5.0f, 4.0f};
  float ballRadius = 20.0f;

  raylib::Color ballColor = MAROON;

  std::cout << "Raylib-CPP Demo Started!" << std::endl;

  // Main game loop
  while (!window.ShouldClose()) {
    // Update
    // Move ball
    ballPosition.x += ballSpeed.x;
    ballPosition.y += ballSpeed.y;

    // Check boundary collision
    if ((ballPosition.x >= (screenWidth - ballRadius)) ||
        (ballPosition.x <= ballRadius)) {
      ballSpeed.x *= -1.0f;
    }
    if ((ballPosition.y >= (screenHeight - ballRadius)) ||
        (ballPosition.y <= ballRadius)) {
      ballSpeed.y *= -1.0f;
    }

#ifndef PLATFORM_ANDROID
    // Non-Android platform input handling
    if (IsKeyPressed(KEY_SPACE)) {
      // Switch ball color
      static int colorIndex = 0;
      raylib::Color colors[] = {MAROON,     ORANGE, DARKGREEN, DARKBLUE,
                                DARKPURPLE, BROWN,  DARKGRAY,  RED};
      colorIndex = (colorIndex + 1) % (sizeof(colors) / sizeof(colors[0]));
      ballColor = colors[colorIndex];
    }
#endif

    // Draw
    window.BeginDrawing();
    {
      window.ClearBackground(RAYWHITE);

      // Draw title
      DrawText("Raylib-CPP Demo", 10, 10, 20, DARKGRAY);

#ifndef PLATFORM_ANDROID
      // Non-Android platform instructions
      DrawText("Press SPACE to change color", 10, 40, 10, GRAY);
      DrawText("Press ESC to exit", 10, 55, 10, GRAY);
#endif

      // Draw bouncing ball
      DrawCircleV(ballPosition, ballRadius, ballColor);

      // Draw some decorative graphics
      DrawRectangle(10, screenHeight - 30, 200, 20, LIGHTGRAY);
      DrawText("Raylib-CPP is awesome!", 15, screenHeight - 25, 10, DARKGRAY);

      // Draw FPS
      DrawFPS(screenWidth - 80, 10);
    }
    window.EndDrawing();
  }

  std::cout << "Demo finished!" << std::endl;
  return 0;
}