/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/extensions/javacrypto/JavaCryptoPeerCert.h>
#include <fizz/extensions/javacrypto/JniUtils.h>
#include <fizz/protocol/Certificate.h>

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

/* static */ Status JavaCryptoPeerCert::create(
    std::unique_ptr<JavaCryptoPeerCert>& ret,
    Error& err,
    Buf certData) {
  bool shouldDetach;
  auto env = jni::getEnv(shouldDetach);
  auto byteArray = jni::createByteArray(env, std::move(certData));
  ret.reset(
      new JavaCryptoPeerCert(env->NewObject(clazz, constructor, byteArray)));
  env->DeleteLocalRef(byteArray);
  FIZZ_RETURN_ON_ERROR(jni::maybeReturnError(err, env, shouldDetach));
  jni::releaseEnv(shouldDetach);
  return Status::Success;
}

std::string JavaCryptoPeerCert::getIdentity() const {
  bool shouldDetach;
  auto env = jni::getEnv(shouldDetach);

  auto jIdentity = (jstring)env->CallObjectMethod(jobject_, getIdentityMethod);
  auto cIdentity = jIdentity
      ? env->GetStringUTFChars(jIdentity, JNI_FALSE /* isCopy */)
      : "";
  std::string identity{cIdentity};
  if (jIdentity) {
    env->ReleaseStringUTFChars(jIdentity, cIdentity);
  }
  Error err;
  FIZZ_THROW_ON_ERROR(jni::maybeReturnError(err, env, shouldDetach), err);
  jni::releaseEnv(shouldDetach);
  return identity;
}

Status JavaCryptoPeerCert::verify(
    Error& err,
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) const {
  bool shouldDetach;
  auto env = jni::getEnv(shouldDetach);

  std::string algorithm;
  switch (scheme) {
    case SignatureScheme::ecdsa_secp256r1_sha256:
      algorithm = "SHA256withECDSA";
      break;
    default:
      return err.error("Unsupported signature scheme");
  }
  auto jAlgorithm = env->NewStringUTF(algorithm.c_str());
  auto signData = fizz::certverify::prepareSignData(context, toBeSigned);
  auto jSignData = jni::createByteArray(env, std::move(signData));
  auto jSignature = jni::createByteArray(env, signature);

  env->CallObjectMethod(
      jobject_, verifyMethod, jAlgorithm, jSignData, jSignature);

  env->DeleteLocalRef(jSignature);
  env->DeleteLocalRef(jSignData);
  env->DeleteLocalRef(jAlgorithm);

  FIZZ_RETURN_ON_ERROR(jni::maybeReturnError(err, env, shouldDetach));
  jni::releaseEnv(shouldDetach);
  return Status::Success;
}

folly::ssl::X509UniquePtr JavaCryptoPeerCert::getX509() const {
  return nullptr;
}

} // namespace fizz
