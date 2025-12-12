#include <raylib.h>

using namespace std;
int main() {
	InitWindow(1300, 800,"窗口标题");
	SetTargetFPS(60);
	
	//读取字体文件
	int fileSize;
	unsigned char *fontFileData = LoadFileData("assets/fonts/a.otf", &fileSize);
	
	//定义要显示的文字
	char text[] = "➜ arrow 😑😶🙄😏😣😥😮🤐😯😪😫😴😌😛😜😝🤤😒😓\n 魑魅魍魉 白日依山尽\n黄河入海流\n欲穷千里目\n更上一层楼\nhello world";
	
	// 将字符串中的字符逐一转换成Unicode码点，得到码点表
	int codepointsCount;
	int *codepoints=LoadCodepoints(text,&codepointsCount);
	// 读取仅码点表中各字符的字体
	Font font = LoadFontFromMemory(".ttf",fontFileData,fileSize,500,codepoints,codepointsCount);
	// 释放码点表
	UnloadCodepoints(codepoints);
	
	while (!WindowShouldClose()) {
		
		BeginDrawing();
		ClearBackground(BLACK);
		
		//把文字画出来
		DrawTextEx(font,text,Vector2{20,20},80,10,DARKBLUE);
		
		EndDrawing();
	}
	
	//千万记得释放资源
	UnloadFont(font);
	
	//关闭窗口
	CloseWindow();
	return 0;
}
