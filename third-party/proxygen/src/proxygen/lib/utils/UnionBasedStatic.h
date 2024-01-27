/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

// This pattern is used to create a static whose destructor will never be
// called. This is useful since some proxygen clients call exit() directly in
// multithreaded programs explicitly against recommendations which lead to
// shutdown crashes due to dependent statics being cleaned up while the threads
// are still executing.

// IMPLEMENTATION MACROS
// (Don't use these directly.)
#define DECLARE_UNION_STATIC_UNION_IMPL(type, name) \
  union name##Union {                               \
    type data;                                      \
    name##Union() {                                 \
    }                                               \
    ~name##Union() {                                \
    }                                               \
  }

#define DECLARE_UNION_STATIC_UNION_ARRAY_IMPL(type, size, name) \
  union name##Union {                                           \
    type data[size];                                            \
    name##Union() {                                             \
    }                                                           \
    ~name##Union() {                                            \
    }                                                           \
  }

#define DEFINE_UNION_STATIC_UNION_IMPL(type, name, var) \
  DECLARE_UNION_STATIC_UNION_IMPL(type, name) var

#define DEFINE_UNION_STATIC_UNION_CONST_IMPL(type, name, var) \
  DECLARE_UNION_STATIC_UNION_IMPL(type, name) const var

#define DEFINE_UNION_STATIC_UNION_CONST_ARRAY_IMPL(type, size, name, var) \
  DECLARE_UNION_STATIC_UNION_ARRAY_IMPL(type, size, name) const var

#if defined(_MSC_VER) && !defined(__clang__)
#define ATTRIBUTE_CONSTRUCTOR
#else
#define ATTRIBUTE_CONSTRUCTOR __attribute__((__constructor__))
#endif

// The const_casts are only needed if creating a const union but it's a
// no-op otherwise so keep it to avoid creating even more macro helpers.
#if defined(_MSC_VER) && !defined(__clang__)
#define DEFINE_UNION_STATIC_CONSTRUCTOR_IMPL(type, name, var) \
  ATTRIBUTE_CONSTRUCTOR                                       \
  void init##name##Union() {                                  \
    new (const_cast<type*>(&var.data)) type();                \
  }                                                           \
                                                              \
  static struct staticInit##name##UnionStruct {               \
    staticInit##name##UnionStruct() {                         \
      init##name##Union();                                    \
    }                                                         \
  } staticInit##name##Union;
#else
#define DEFINE_UNION_STATIC_CONSTRUCTOR_IMPL(type, name, var) \
  ATTRIBUTE_CONSTRUCTOR                                       \
  void init##name##Union() {                                  \
    new (const_cast<type*>(&var.data)) type();                \
  }
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define DEFINE_UNION_STATIC_CONSTRUCTOR_ARG_IMPL(type, name, var, ...) \
  ATTRIBUTE_CONSTRUCTOR                                                \
  void init##name##Union() {                                           \
    new (const_cast<type*>(&var.data)) type(__VA_ARGS__);              \
  }                                                                    \
                                                                       \
  static struct staticInit##name##UnionStruct {                        \
    staticInit##name##UnionStruct() {                                  \
      init##name##Union();                                             \
    }                                                                  \
  } staticInit##name##Union;
#else
#define DEFINE_UNION_STATIC_CONSTRUCTOR_ARG_IMPL(type, name, var, ...) \
  ATTRIBUTE_CONSTRUCTOR                                                \
  void init##name##Union() {                                           \
    new (const_cast<type*>(&var.data)) type(__VA_ARGS__);              \
  }
#endif
// END IMPLEMENTATION MACROS

// Use var.data to access the actual member of interest. Zero and argument
// versions are provided. If you need to do custom construction, like using a
// brace-enclosed initializer list, use the NO_INIT variant and then define
// your own __attribute__((__constructor__)) function to do the initialization.
#define DEFINE_UNION_STATIC(type, name, var)       \
  DEFINE_UNION_STATIC_UNION_IMPL(type, name, var); \
  DEFINE_UNION_STATIC_CONSTRUCTOR_IMPL(type, name, var)

#define DEFINE_UNION_STATIC_ARGS(type, name, var, ...) \
  DEFINE_UNION_STATIC_UNION_IMPL(type, name, var);     \
  DEFINE_UNION_STATIC_CONSTRUCTOR_ARG_IMPL(type, name, var, __VA_ARGS__)

#define DEFINE_UNION_STATIC_NO_INIT(type, name, var) \
  DEFINE_UNION_STATIC_UNION_IMPL(type, name, var)

// Same as the above three except used to create a const union.
#define DEFINE_UNION_STATIC_CONST(type, name, var)       \
  DEFINE_UNION_STATIC_UNION_CONST_IMPL(type, name, var); \
  DEFINE_UNION_STATIC_CONSTRUCTOR_IMPL(type, name, var)

#define DEFINE_UNION_STATIC_CONST_ARGS(type, name, var, ...) \
  DEFINE_UNION_STATIC_UNION_CONST_IMPL(type, name, var);     \
  DEFINE_UNION_STATIC_CONSTRUCTOR_ARG_IMPL(type, name, var, __VA_ARGS__)

#define DEFINE_UNION_STATIC_CONST_NO_INIT(type, name, var) \
  DEFINE_UNION_STATIC_UNION_CONST_IMPL(type, name, var)

#define DEFINE_UNION_STATIC_CONST_ARRAY_NO_INIT(type, size, name, var) \
  DEFINE_UNION_STATIC_UNION_CONST_ARRAY_IMPL(type, size, name, var)

// Use these if you need to extern one of these in a header and then
// define it in a .cpp file. For example:
//
// Header:
// DECLARE_UNION_STATIC(std::string, StdString);
// extern const StdStringUnion kStringConstant;
//
// Definition:
// const IMPLEMENT_DECLARED_UNION_STATIC_ARGS(
//   std::string, StdString, DoesNotMatter, kStringConstant, "hello world");
//
#define DECLARE_UNION_STATIC(type, name) \
  DECLARE_UNION_STATIC_UNION_IMPL(type, name)

#define IMPLEMENT_DECLARED_UNION_STATIC(type, unionName, name, var) \
  unionName##Union var;                                             \
  DEFINE_UNION_STATIC_CONSTRUCTOR_ARG_IMPL(type, name, var)

#define IMPLEMENT_DECLARED_UNION_STATIC_ARGS(type, unionName, name, var, ...) \
  unionName##Union var;                                                       \
  DEFINE_UNION_STATIC_CONSTRUCTOR_ARG_IMPL(type, name, var, __VA_ARGS__)
