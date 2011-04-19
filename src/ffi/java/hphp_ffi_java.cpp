/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
/*
 * hphp_ffi_java.cpp
 *
 *  Created on: Sep 13, 2009
 *      Author: qixin
 */

#include <jni.h>
#include <vector>
#include <runtime/base/hphp_ffi.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/string_data.h>
#include <runtime/base/program_functions.h>
#include <runtime/base/runtime_option.h>
#include "hphp_ffi_java.h"

using namespace HPHP;
using namespace std;

ExecutionContext *context = NULL;
vector <Variant *> pointers;

/**
 * Helper function that creates HphpVariant objects in Java.
 */
jobject exportVariantToJava(JNIEnv *env, jclass hphp, void *value, int kind) {
  Variant *result;
  switch (kind) {
    case 0:
      static jmethodID null_init =
          env->GetStaticMethodID(hphp, "createHphpNull",
                                 "(J)Lhphp/HphpNull;");
      result = hphp_ffi_buildVariant(0, 0, 0);
      pointers.push_back(result);
      return env->CallStaticObjectMethod(hphp, null_init, (jlong)result);
    case 1:
      static jmethodID boolean_init =
          env->GetStaticMethodID(hphp, "createHphpBoolean",
                                 "(JZ)Lhphp/HphpBoolean;");
      result = hphp_ffi_buildVariant(1, value, 0);
      pointers.push_back(result);
      return env->CallStaticObjectMethod(hphp, boolean_init, (jlong)result,
                                         value != NULL);
    case 2:
      static jmethodID int64_init =
          env->GetStaticMethodID(hphp, "createHphpInt64",
                                 "(JJ)Lhphp/HphpInt64;");
      result = hphp_ffi_buildVariant(2, value, 0);
      pointers.push_back(result);
      return env->CallStaticObjectMethod(hphp, int64_init, (jlong)result,
                                         (jlong)value);
    case 3:
      static jmethodID double_init =
          env->GetStaticMethodID(hphp, "createHphpDouble",
                                 "(JD)Lhphp/HphpDouble;");
      result = hphp_ffi_buildVariant(3, value, 0);
      pointers.push_back(result);
      union {
        double d;
        void *p;
      } u;
      u.p = value;
      return env->CallStaticObjectMethod(hphp, double_init, (jlong)result,
                                         u.d);
    case 4:
      value = NEW(StringData)((const char *)value, CopyString);
    case 5:
      static jmethodID string_init =
          env->GetStaticMethodID(hphp, "createHphpString",
                                 "(J)Lhphp/HphpString;");
      result = hphp_ffi_buildVariant(5, value, 0);
      pointers.push_back(result);
      return env->CallStaticObjectMethod(hphp, string_init, (jlong)result);
    case 6:
      static jmethodID array_init =
          env->GetStaticMethodID(hphp, "createHphpArray",
                                 "(J)Lhphp/HphpArray;");
      result = hphp_ffi_buildVariant(7, value, 0);
      pointers.push_back(result);
      return env->CallStaticObjectMethod(hphp, array_init, (jlong)result);
    case 7:
      static jmethodID object_init =
          env->GetStaticMethodID(hphp, "createHphpObject",
                                 "(J)Lhphp/HphpObject;");
      result = hphp_ffi_buildVariant(8, value, 0);
      pointers.push_back(result);
      return env->CallStaticObjectMethod(hphp, object_init, (jlong)result);
    default:
      return NULL;
  }
}

extern "C" {

/* Header for class hphp_Hphp */

/*
 * Class:     hphp_Hphp
 * Method:    buildVariant
 * Signature: (IJI)J
 */
JNIEXPORT jlong JNICALL
Java_hphp_Hphp_buildVariant(JNIEnv *env, jclass hphp,
                            jint kind, jlong v, jint len) {
  Variant *result = hphp_ffi_buildVariant(kind, (void *)v, len);
  pointers.push_back(result);
  return (jlong)result;
}

/*
 * Class:     hphp_Hphp
 * Method:    freeVariant
 * Signature: (J)V
 */
JNIEXPORT void JNICALL
Java_hphp_Hphp_freeVariant(JNIEnv *env, jclass hphp, jlong v) {
  hphp_ffi_freeVariant((Variant *)v);
}

/*
 * Class:     hphp_Hphp
 * Method:    getIterBegin
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL
Java_hphp_Hphp_getIterBegin(JNIEnv *env, jclass hphp, jlong ptr) {
  return hphp_ffi_iter_begin(((Variant *)ptr)->getArrayData());
}

/*
 * Class:     hphp_Hphp
 * Method:    getIterAdvanced
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL
Java_hphp_Hphp_getIterAdvanced(JNIEnv *env, jclass hphp, jlong ptr,
                               jlong pos) {
  return hphp_ffi_iter_advance(((Variant *)ptr)->getArrayData(), pos);
}

/*
 * Class:     hphp_Hphp
 * Method:    isIterValid
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL
Java_hphp_Hphp_isIterValid(JNIEnv *env, jclass hphp, jlong ptr, jlong pos) {
  return !(hphp_ffi_iter_invalid(pos));
}

/*
 * Class:     hphp_Hphp
 * Method:    getKey
 * Signature: (JJ)Lhphp/HphpVariant;
 */
JNIEXPORT jobject JNICALL
Java_hphp_Hphp_getKey(JNIEnv *env, jclass hphp, jlong arrPtr, jlong pos) {
  void *result;
  int kind = hphp_ffi_iter_getKey(((Variant *)arrPtr)->getArrayData(), pos,
                                  &result);
  return exportVariantToJava(env, hphp, result, kind);
}

/*
 * Class:     hphp_Hphp
 * Method:    getValue
 * Signature: (JJ)Lhphp/HphpVariant;
 */
JNIEXPORT jobject JNICALL
Java_hphp_Hphp_getValue(JNIEnv *env, jclass hphp, jlong arrPtr, jlong pos) {
  void *result;
  int kind = hphp_ffi_iter_getValue(((Variant *)arrPtr)->getArrayData(), pos,
                                    &result);
  return exportVariantToJava(env, hphp, result, kind);
}

/*
 * Class:     hphp_Hphp
 * Method:    includeFile
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_hphp_Hphp_includeFile(JNIEnv *env, jclass hphp, jstring file) {
  const char *fstr = env->GetStringUTFChars(file, NULL);
  hphp_ffi_include_file(fstr);
  env->ReleaseStringUTFChars(file, fstr);
}

/*
 * Class:     hphp_Hphp
 * Method:    invokeFunction
 * Signature: (Ljava/lang/String;J)Lhphp/HphpVariant;
 */
JNIEXPORT jobject JNICALL
Java_hphp_Hphp_invokeFunction(JNIEnv *env, jclass hphp, jstring func,
                              jlong argsPtr) {
  const char *fstr = env->GetStringUTFChars(func, NULL);
  void *result;
  int kind =
      hphp_ffi_invoke_function(&result, fstr,
                               ((Variant *)argsPtr)->getArrayData());
  env->ReleaseStringUTFChars(func, fstr);
  return exportVariantToJava(env, hphp, result, kind);
}

/*
 * Class:     hphp_Hphp
 * Method:    invokeStaticMethod
 * Signature: (Ljava/lang/String;Ljava/lang/String;J)Lhphp/HphpVariant;
 */
JNIEXPORT jobject JNICALL
Java_hphp_Hphp_invokeStaticMethod(JNIEnv *env, jclass hphp, jstring cls,
                                  jstring func, jlong argsPtr) {
  const char *cstr = env->GetStringUTFChars(cls, NULL);
  const char *fstr = env->GetStringUTFChars(func, NULL);
  void *result;
  int kind =
      hphp_ffi_invoke_static_method(&result, cstr, fstr,
                                    ((Variant *)argsPtr)->getArrayData());
  env->ReleaseStringUTFChars(cls, cstr);
  env->ReleaseStringUTFChars(func, fstr);
  return exportVariantToJava(env, hphp, result, kind);
}

/*
 * Class:     hphp_Hphp
 * Method:    invokeMethod
 * Signature: (JLjava/lang/String;J)Lhphp/HphpVariant;
 */
JNIEXPORT jobject JNICALL
Java_hphp_Hphp_invokeMethod(JNIEnv *env, jclass hphp, jlong targetPtr,
                            jstring func, jlong argsPtr) {
  const char *fstr = env->GetStringUTFChars(func, NULL);
  void *result;
  int kind =
      hphp_ffi_invoke_object_method(&result,
                                    ((Variant *)targetPtr)->getObjectData(),
                                    fstr,
                                    ((Variant *)argsPtr)->getArrayData());
  env->ReleaseStringUTFChars(func, fstr);
  return exportVariantToJava(env, hphp, result, kind);
}

/*
 * Class:     hphp_Hphp
 * Method:    createObject
 * Signature: (Ljava/lang/String;J)Lhphp/HphpVariant;
 */
JNIEXPORT jobject JNICALL
Java_hphp_Hphp_createObject(JNIEnv *env, jclass hphp, jstring cls,
                            jlong argsPtr) {
  const char *cstr = env->GetStringUTFChars(cls, NULL);
  void *result;
  int kind =
      hphp_ffi_create_object(&result, cstr,
                             ((Variant *)argsPtr)->getArrayData());
  env->ReleaseStringUTFChars(cls, cstr);
  return exportVariantToJava(env, hphp, result, kind);
}

/*
 * Class:     hphp_Hphp
 * Method:    set
 * Signature: (JIJIJI)V
 */
JNIEXPORT void JNICALL
Java_hphp_Hphp_set(JNIEnv *env, jclass hphp, jlong map, jlong key,
                   jlong value) {
  hphp_ffi_addMapItem((Variant *)map, (Variant *)key, (Variant *)value);
}

/*
 * Class:     hphp_Hphp
 * Method:    get
 * Signature: (JJ)Lhphp/HphpVariant;
 */
JNIEXPORT jobject JNICALL
Java_hphp_Hphp_get(JNIEnv *env, jclass hphp, jlong map, jlong key) {
  void *result;
  int kind = hphp_ffi_getMapItem(&result, (Variant *)map, (Variant *)key);
  return exportVariantToJava(env, hphp, result, kind);
}

/*
 * Class:     hphp_Hphp
 * Method:    createHphpString
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL
Java_hphp_Hphp_createHphpString(JNIEnv *env, jclass hphp, jstring str) {
  const char *s = env->GetStringUTFChars(str, NULL);
  jsize len = env->GetStringUTFLength(str);

  // make a copy of the underlying string to avoid GC issue
  Variant *result = hphp_ffi_buildVariant(6, (void *)s, len);
  pointers.push_back(result);

  env->ReleaseStringUTFChars(str, s);

  return (jlong)result;
}

/*
 * Class:     hphp_Hphp
 * Method:    ffiProcessInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_hphp_Hphp_ffiProcessInit(JNIEnv *env, jclass hphp) {
  hphp_ffi_init();
}

/*
 * Class:     hphp_Hphp
 * Method:    ffiSessionInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_hphp_Hphp_ffiSessionInit(JNIEnv *env, jclass hphp) {
  hphp_ffi_session_init();
  context = hphp_ffi_context_init();
}

/*
 * Class:     hphp_Hphp
 * Method:    ffiSessionExit
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_hphp_Hphp_ffiSessionExit(JNIEnv *env, jclass hphp) {
  for (typeof(pointers.begin()) it = pointers.begin(); it != pointers.end();
       it++) {
    hphp_ffi_freeVariant(*it);
  }
  pointers.clear();
  hphp_ffi_context_exit(context);
  context = NULL;
  hphp_ffi_session_exit();
}

/* Header for class hphp_HphpString */

/*
 * Class:     hphp_HphpString
 * Method:    getHphpString
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_hphp_HphpString_getHphpString(JNIEnv *env, jobject str, jlong ptr) {
  const char *cstr;
  hphp_ffi_string_data(((Variant *)ptr)->getStringData(), &cstr);
  return env->NewStringUTF(cstr);
}

/*
 * Class:     hphp_HphpArray
 * Method:    append
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL
Java_hphp_HphpArray_append(JNIEnv *env, jobject arr, jlong arrPtr,
                           jlong valPtr) {
  ((Variant *)arrPtr)->append(*(Variant *)valPtr);
}

}
