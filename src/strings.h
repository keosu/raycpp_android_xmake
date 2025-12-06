#pragma once
#include <string>
#include <map>

// 标签枚举（全局定义）
enum DemoTab {
    TAB_SHAPES = 0,
    TAB_TEXT,
    TAB_COLORS,
    TAB_INPUT,
    TAB_ANIMATION,
    TAB_3D,
    TAB_COUNT
};

// 所有UI文本的集中管理，便于国际化和字体加载
namespace Strings {

// 特性信息结构
struct FeatureText {
    const char* nameEN;
    const char* nameCN;
    const char* descEN;
    const char* descCN;
    const char* usageEN;
    const char* usageCN;
};

// 使用map存储标签文本
const std::map<DemoTab, const char*> TAB_NAMES_CN = {
    {TAB_SHAPES, "图形"},
    {TAB_TEXT, "文本"},
    {TAB_COLORS, "颜色"},
    {TAB_INPUT, "输入"},
    {TAB_ANIMATION, "动画"},
    {TAB_3D, "3D"}
};

const std::map<DemoTab, const char*> TAB_NAMES_EN = {
    {TAB_SHAPES, "Shapes"},
    {TAB_TEXT, "Text"},
    {TAB_COLORS, "Colors"},
    {TAB_INPUT, "Input"},
    {TAB_ANIMATION, "Animation"},
    {TAB_3D, "3D"}
};

// 使用map存储图标（不再使用，已使用纹理）
const std::map<DemoTab, const char*> TAB_ICONS = {
    {TAB_SHAPES, "■"},
    {TAB_TEXT, "字"},
    {TAB_COLORS, "◐"},
    {TAB_INPUT, "✎"},
    {TAB_ANIMATION, "◉"},
    {TAB_3D, "◆"}
};

// 使用map存储特性信息
const std::map<DemoTab, FeatureText> FEATURES = {
    {TAB_SHAPES, {
        "2D Shapes Drawing",
        "2D 图形绘制",
        "Draw various geometric shapes with colors and outlines",
        "绘制各种几何图形，支持颜色填充和轮廓",
        "Circles, rectangles, lines, polygons, anti-aliased rendering",
        "圆形、矩形、线条、多边形、抗锯齿渲染"
    }},
    {TAB_TEXT, {
        "Text Rendering",
        "文本渲染",
        "Display text with custom fonts and styles",
        "使用自定义字体和样式显示文本",
        "Multiple fonts, sizes, colors, Unicode support",
        "多字体、大小、颜色、Unicode 支持"
    }},
    {TAB_COLORS, {
        "Colors & Gradients",
        "颜色与渐变",
        "Work with colors, blending and gradient effects",
        "处理颜色、混合和渐变效果",
        "Color manipulation, gradients, transparency",
        "颜色处理、渐变、透明度"
    }},
    {TAB_INPUT, {
        "Input Handling",
        "输入处理",
        "Mouse and keyboard input detection",
        "鼠标和键盘输入检测",
        "Mouse position, clicks, key presses, real-time interaction",
        "鼠标位置、点击、按键、实时交互"
    }},
    {TAB_ANIMATION, {
        "Animation & Particles",
        "动画与粒子",
        "Create dynamic animations and particle effects",
        "创建动态动画和粒子效果",
        "Smooth transitions, particle systems, visual effects",
        "平滑过渡、粒子系统、视觉效果"
    }},
    {TAB_3D, {
        "3D Graphics",
        "3D 图形",
        "Basic 3D rendering and camera control",
        "基础 3D 渲染和摄像机控制",
        "3D models, lighting, camera, transformations",
        "3D 模型、光照、摄像机、变换"
    }}
};

// 主标题
const char* APP_TITLE = "Raylib 特性演示 Features Demo";
const char* APP_SUBTITLE = "Raylib 基础功能展示 Basic Features Showcase";

// UI提示
const char* INFO_HINT = "Press I for info";
const char* INFO_HINT_MOBILE = "双击显示详情";
const char* INFO_LABEL = "特性 Features:";

// 演示页面文本
const char* SHAPES_TITLE = "2D 图形绘制 Shapes Drawing";
const char* SHAPES_DESC = "各种几何图形、线条和抗锯齿渲染";
const char* SHAPES_BASIC = "基本图形 Basic Shapes";
const char* SHAPES_ADVANCED = "渐变与线条 Gradients & Lines";
const char* SHAPES_CIRCLE = "Circle";
const char* SHAPES_RECTANGLE = "Rectangle";
const char* SHAPES_TRIANGLE = "Triangle";
const char* SHAPES_GRADIENT = "Gradient";
const char* SHAPES_RING = "Ring (AA)";
const char* SHAPES_LINES = "Lines";

const char* TEXT_TITLE = "文本渲染 Text Rendering";
const char* TEXT_DESC = "自定义字体、大小、颜色和 Unicode 支持";
const char* TEXT_DEFAULT = "Default Font - Raylib 默认字体";
const char* TEXT_DEFAULT_SMALL = "Default Font - Small Size 小号";
const char* TEXT_CUSTOM_LARGE = "自定义字体 Custom Font - 大号 Large";
const char* TEXT_CUSTOM_MEDIUM = "自定义字体 中号 Medium";
const char* TEXT_CUSTOM_SMALL = "自定义字体 小号 Small";
const char* TEXT_UNICODE = "Unicode: 中文 日本語 한국어 Русский";
const char* TEXT_COLORED = "彩色文字渲染 Colored Text";
const char* TEXT_ANIMATED = "动画文本 Animated Text";

const char* COLORS_TITLE = "颜色与渐变 Colors & Gradients";
const char* COLORS_DESC = "颜色处理、渐变效果和透明度控制";
const char* COLORS_BASIC = "基本颜色 Basic Colors:";
const char* COLORS_RAINBOW = "HSV 彩虹 Rainbow:";
const char* COLORS_GRADIENT = "渐变 Gradients:";
const char* COLORS_TRANSPARENCY = "透明度 Transparency:";
const char* COLORS_VERTICAL = "Vertical";
const char* COLORS_HORIZONTAL = "Horizontal";
const char* COLORS_RADIAL = "Radial";

const char* INPUT_TITLE = "输入处理 Input Handling";
const char* INPUT_DESC = "鼠标和键盘输入的实时检测";
const char* INPUT_MOUSE = "鼠标信息 Mouse Info:";
const char* INPUT_KEYBOARD = "键盘信息 Keyboard Info:";
const char* INPUT_INTERACTIVE = "交互区域 Interactive Area:";
const char* INPUT_POSITION = "位置 Position:";
const char* INPUT_LEFT_BUTTON = "左键 Left Button:";
const char* INPUT_WHEEL = "滚轮 Wheel:";
const char* INPUT_PRESS_KEY = "按下任意键测试 Press any key to test";
const char* INPUT_LAST_KEY = "最后按键 Last Key:";
const char* INPUT_CLICK_DRAW = "点击鼠标在此区域绘制 Click to draw";
const char* INPUT_BUTTON_DOWN = "按下 Down";
const char* INPUT_BUTTON_UP = "释放 Up";

const char* ANIM_TITLE = "动画与粒子 Animation & Particles";
const char* ANIM_DESC = "动态动画、平滑过渡和粒子系统";
const char* ANIM_PARTICLES = "粒子数量 Particles:";
const char* ANIM_CLICK_CREATE = "点击鼠标创建粒子 Click to create particles";
const char* ANIM_WAVE = "波浪动画 Wave Animation";

const char* TD_TITLE = "3D 图形 3D Graphics";
const char* TD_DESC = "基础 3D 渲染和摄像机控制";

const char* TD3_TITLE = "3D 图形 3D Graphics";
const char* TD3_DESC = "基础 3D 渲染和摄像机控制";
const char* TD3_CAMERA = "摄像机 Camera:";
const char* TD3_ROTATION = "旋转 Rotation:";

// 颜色名称
const char* COLOR_NAMES[] = {"红", "橙", "黄", "绿", "青", "蓝", "紫", "粉"};

// 收集所有需要加载的中文字符（从所有const char*字符串中提取）
std::string GetAllChineseChars() {
    std::string chars;
    
    // 从TAB_NAMES_CN收集
    for (const auto& pair : TAB_NAMES_CN) {
        chars += pair.second;
    }
    
    // 从FEATURES收集
    for (const auto& pair : FEATURES) {
        const FeatureText& ft = pair.second;
        chars += ft.nameCN;
        chars += ft.descCN;
        chars += ft.usageCN;
    }
    
    // 添加其他UI文本
    chars += APP_TITLE;
    chars += APP_SUBTITLE;
    chars += INFO_HINT_MOBILE;
    chars += INFO_LABEL;
    
    // 演示页面文本
    chars += SHAPES_TITLE;
    chars += SHAPES_DESC;
    chars += SHAPES_BASIC;
    chars += SHAPES_ADVANCED;
    
    chars += TEXT_TITLE;
    chars += TEXT_DESC;
    chars += TEXT_DEFAULT;
    chars += TEXT_DEFAULT_SMALL;
    chars += TEXT_CUSTOM_LARGE;
    chars += TEXT_CUSTOM_MEDIUM;
    chars += TEXT_CUSTOM_SMALL;
    chars += TEXT_UNICODE;
    chars += TEXT_COLORED;
    chars += TEXT_ANIMATED;
    
    chars += COLORS_TITLE;
    chars += COLORS_DESC;
    chars += COLORS_BASIC;
    chars += COLORS_RAINBOW;
    chars += COLORS_GRADIENT;
    chars += COLORS_TRANSPARENCY;
    chars += COLORS_VERTICAL;
    chars += COLORS_HORIZONTAL;
    chars += COLORS_RADIAL;
    
    chars += INPUT_TITLE;
    chars += INPUT_DESC;
    chars += INPUT_MOUSE;
    chars += INPUT_KEYBOARD;
    chars += INPUT_INTERACTIVE;
    chars += INPUT_POSITION;
    chars += INPUT_LEFT_BUTTON;
    chars += INPUT_WHEEL;
    chars += INPUT_PRESS_KEY;
    chars += INPUT_LAST_KEY;
    chars += INPUT_CLICK_DRAW;
    chars += INPUT_BUTTON_DOWN;
    chars += INPUT_BUTTON_UP;
    
    chars += ANIM_TITLE;
    chars += ANIM_DESC;
    chars += ANIM_PARTICLES;
    chars += ANIM_CLICK_CREATE;
    chars += ANIM_WAVE;
    
    chars += TD3_TITLE;
    chars += TD3_DESC;
    chars += TD3_CAMERA;
    chars += TD3_ROTATION;
    
    // 颜色名称
    for (int i = 0; i < 8; i++) {
        chars += COLOR_NAMES[i];
    }
    
    // 日志相关
    chars += "尝试加载字体文件存在开始个常用字符成功失败不跳过"
            "字形数量错误无法确保以任一于目录建议操作"
            "运行下载或手到编译安装程序将退出"
            "提示未使替代如需支持请切换全屏模式退出进入";
    
    return chars;
}

} // namespace Strings

