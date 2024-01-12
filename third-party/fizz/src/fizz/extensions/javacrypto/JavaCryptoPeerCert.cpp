/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/extensions/javacrypto/JavaCryptoPeerCert.h>
#include <fizz/extensions/javacrypto/JniUtils.h>
#include <fizz/protocol/CertUtils.h>

namespace fizz {

namespace {
jclass clazz;
jmethodID constructor;
jmethodID getIdentityMethod;
jmethodID verifyMethod;
} // namespace

void JavaCryptoPeerCert::onLoad(JNIEnv* env) {
  clazz = jni::getClass(env, "com/facebook/fizz/JavaCryptoPeerCert");
  constructor = jni::getMethodID(env, clazz, "<init>", "([B)V");
  getIdentityMethod =
      jni::getMethodID(env, clazz, "getIdentity", "()Ljava/lang/String;");
  verifyMethod =
      jni::getMethodID(env, clazz, "verify", "(Ljava/lang/String;[B[B)V");
}

JavaCryptoPeerCert::JavaCryptoPeerCert(Buf certData) {
  bool shouldDetach;
  auto env = jni::getEnv(&shouldDetach);

  auto byteArray = jni::createByteArray(env, std::move(certData));
  jobject_ = env->NewObject(clazz, constructor, byteArray);
  env->DeleteLocalRef(byteArray);

  jni::maybeThrowException(env, shouldDetach);
  jni::releaseEnv(shouldDetach);
}

std::string JavaCryptoPeerCert::getIdentity() const {
  bool shouldDetach;
  auto env = jni::getEnv(&shouldDetach);

  auto jIdentity = (jstring)env->CallObjectMethod(jobject_, getIdentityMethod);
  auto cIdentity = env->GetStringUTFChars(jIdentity, JNI_FALSE /* isCopy */);
  std::string identity{cIdentity};
  env->ReleaseStringUTFChars(jIdentity, cIdentity);

  jni::maybeThrowException(env, shouldDetach);
  jni::releaseEnv(shouldDetach);
  return identity;
}

void JavaCryptoPeerCert::verify(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) const {
  bool shouldDetach;
  auto env = jni::getEnv(&shouldDetach);

  std::string algorithm;
  switch (scheme) {
    case SignatureScheme::ecdsa_secp256r1_sha256:
      algorithm = "SHA256withECDSA";
      break;
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
  auto jAlgorithm = env->NewStringUTF(algorithm.c_str());
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  auto jSignData = jni::createByteArray(env, std::move(signData));
  auto jSignature = jni::createByteArray(env, signature);

  env->CallObjectMethod(
      jobject_, verifyMethod, jAlgorithm, jSignData, jSignature);

  env->DeleteLocalRef(jSignature);
  env->DeleteLocalRef(jSignData);
  env->DeleteLocalRef(jAlgorithm);

  jni::maybeThrowException(env, shouldDetach);
  jni::releaseEnv(shouldDetach);
}

folly::ssl::X509UniquePtr JavaCryptoPeerCert::getX509() const {
  return nullptr;
}

} // namespace fizz
