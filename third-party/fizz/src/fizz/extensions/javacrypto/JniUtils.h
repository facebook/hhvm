/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Types.h>
#include <folly/Range.h>
#include <jni.h>

namespace fizz {
namespace jni {

void setVM(JavaVM* vm);

JNIEnv* getEnv(bool* shouldDetach);
void releaseEnv(bool shouldDetach);

jclass getClass(JNIEnv* env, const std::string& name);

jmethodID getMethodID(
    JNIEnv* env,
    jclass clazz,
    const std::string& name,
    const std::string& signature);

void maybeThrowException(JNIEnv* env, bool shouldDetach);

jbyteArray createByteArray(JNIEnv* env, folly::ByteRange byteRange);
jbyteArray createByteArray(JNIEnv* env, Buf buf);

} // namespace jni
} // namespace fizz
