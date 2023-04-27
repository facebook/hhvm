/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/extensions/javacrypto/JniUtils.h>
#include <glog/logging.h>

namespace fizz {
namespace jni {

namespace {
JavaVM* vm;
}

void setVM(JavaVM* jvm) {
  vm = jvm;
}

JNIEnv* getEnv(bool* shouldDetach) {
  *shouldDetach = false;

  JNIEnv* env;
  auto status = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
  CHECK(status == JNI_OK || status == JNI_EDETACHED);

  if (status == JNI_EDETACHED) {
    status = vm->AttachCurrentThread(
        reinterpret_cast<void**>(&env), nullptr /*args*/);
    CHECK_EQ(status, JNI_OK);
    *shouldDetach = true;
  }

  return env;
}

void releaseEnv(bool shouldDetach) {
  if (shouldDetach) {
    vm->DetachCurrentThread();
  }
}

jclass getClass(JNIEnv* env, const std::string& name) {
  auto clazz =
      reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass(name.c_str())));
  CHECK(clazz);
  return clazz;
}

jmethodID getMethodID(
    JNIEnv* env,
    jclass clazz,
    const std::string& name,
    const std::string& signature) {
  auto methodId = env->GetMethodID(clazz, name.c_str(), signature.c_str());
  CHECK(methodId);
  return methodId;
}

void maybeThrowException(JNIEnv* env, bool shouldDetach) {
  if (!env->ExceptionCheck()) {
    return;
  }
  env->ExceptionDescribe();
  releaseEnv(shouldDetach);
  throw std::runtime_error("JNI exception");
}

jbyteArray createByteArray(JNIEnv* env, folly::ByteRange byteRange) {
  auto byteArray = env->NewByteArray(byteRange.size());
  env->SetByteArrayRegion(
      byteArray,
      0 /*start*/,
      byteRange.size(),
      reinterpret_cast<const jbyte*>(byteRange.data()));
  return byteArray;
}

jbyteArray createByteArray(JNIEnv* env, Buf buf) {
  return createByteArray(env, buf->coalesce());
}

} // namespace jni
} // namespace fizz
