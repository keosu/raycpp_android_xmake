// #ifdef PLATFORM_ANDROID
#include <jni.h>

// 声明main函数
extern "C" int main();

// JNI接口函数
extern "C" JNIEXPORT void JNICALL
Java_com_ray_yi_NativeLoader_main(JNIEnv *env, jclass clazz) {
  // 调用main函数
  main();
}
// #endif
