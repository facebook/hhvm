/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/extensions/javacrypto/JavaCryptoPeerCert.h>
#include <fizz/extensions/javacrypto/JniUtils.h>

using namespace fizz;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return -1;
  }

  jni::setVM(vm);
  JavaCryptoPeerCert::onLoad(env);

  return JNI_VERSION_1_6;
}
