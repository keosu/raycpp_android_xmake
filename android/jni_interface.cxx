
#include <jni.h>

// 声明main函数
extern "C" int main();

// JNI接口函数
extern "C" JNIEXPORT void JNICALL
Java_com_raylib_demo_NativeLoader_main(JNIEnv *env, jclass clazz) {
  // 调用main函数
  main();
}
