/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 */

#pragma once

namespace fizz {

#define FIZZ_UNION_TYPE(X, ...) X X##_;

#define FIZZ_ENUM_TYPES(X, ...) X##_E,

#define FIZZ_UNION_ACCESSOR(X, ...)                       \
  X* as##X() {                                            \
    if (type_ == Type::X##_E) {                           \
      return &X##_;                                       \
    }                                                     \
    return nullptr;                                       \
  }                                                       \
                                                          \
  X& tryAs##X() {                                         \
    auto ptr = as##X();                                   \
    if (!ptr) {                                           \
      throw std::runtime_error("Mismatched access type"); \
    }                                                     \
    return *ptr;                                          \
  }

#define FIZZ_CONST_UNION_ACCESSOR(X, ...)                 \
  const X* as##X() const {                                \
    if (type_ == Type::X##_E) {                           \
      return &X##_;                                       \
    }                                                     \
    return nullptr;                                       \
  }                                                       \
                                                          \
  const X& tryAs##X() const {                             \
    auto ptr = as##X();                                   \
    if (!ptr) {                                           \
      throw std::runtime_error("Mismatched access type"); \
    }                                                     \
    return *ptr;                                          \
  }

#define FIZZ_UNION_CTORS(X, NAME)    \
  NAME(X&& x) : type_(Type::X##_E) { \
    new (&X##_) X(std::move(x));     \
  }

#define FIZZ_UNION_COPY_CTORS(X, NAME)    \
  NAME(const X& x) : type_(Type::X##_E) { \
    new (&X##_) X(x);                     \
  }

#define FIZZ_UNION_MOVE_CASES(X, other)   \
  case Type::X##_E:                       \
    new (&X##_) X(std::move(other.X##_)); \
    break;

#define FIZZ_UNION_COPY_CASES(X, other) \
  case Type::X##_E:                     \
    new (&X##_) X(other.X##_);          \
    break;

// we are using partial instantiation to avoid instantiating these until someone
// calls getType
#define FIZZ_UNION_TYPED_GET(X, ...)          \
  template <class Extra>                      \
  struct TypedGet<X, Extra> {                 \
    template <class Type>                     \
    static X* get(Type& t) {                  \
      return t.as##X();                       \
    }                                         \
    template <class Type>                     \
    static const X* getConst(const Type& t) { \
      return t.as##X();                       \
    }                                         \
  };

#define FIZZ_DESTRUCTOR_CASES(X, ...) \
  case Type::X##_E:                   \
    X##_.~X();                        \
    break;

#define FIZZ_UNION_EQUALITY_CASES(X, other) \
  case Type::X##_E:                         \
    return X##_ == *other.as##X();

#define FIZZ_DECLARE_VARIANT_TYPE(NAME, X)                     \
  struct NAME {                                                \
    enum class Type { X(FIZZ_ENUM_TYPES) };                    \
                                                               \
    X(FIZZ_UNION_CTORS, NAME)                                  \
                                                               \
    NAME(NAME&& other) {                                       \
      switch (other.type_) { X(FIZZ_UNION_MOVE_CASES, other) } \
      type_ = other.type_;                                     \
    }                                                          \
                                                               \
    NAME& operator=(NAME&& other) {                            \
      destroyVariant();                                        \
      switch (other.type_) { X(FIZZ_UNION_MOVE_CASES, other) } \
      type_ = other.type_;                                     \
      return *this;                                            \
    }                                                          \
                                                               \
    ~NAME() {                                                  \
      destroyVariant();                                        \
    }                                                          \
                                                               \
    Type type() const {                                        \
      return type_;                                            \
    }                                                          \
                                                               \
    X(FIZZ_UNION_ACCESSOR)                                     \
                                                               \
    X(FIZZ_CONST_UNION_ACCESSOR)                               \
                                                               \
    template <class T, class E>                                \
    struct TypedGet {};                                        \
    X(FIZZ_UNION_TYPED_GET)                                    \
                                                               \
    /* Do not use in non-test code */                          \
    template <class T>                                         \
    T* getType() {                                             \
      return TypedGet<T, int>::get(*this);                     \
    }                                                          \
    template <class T>                                         \
    const T* getType() const {                                 \
      return TypedGet<T, int>::getConst(*this);                \
    }                                                          \
                                                               \
   private:                                                    \
    union {                                                    \
      X(FIZZ_UNION_TYPE)                                       \
    };                                                         \
                                                               \
    void destroyVariant() {                                    \
      switch (type_) { X(FIZZ_DESTRUCTOR_CASES) }              \
    }                                                          \
                                                               \
    Type type_;                                                \
  };

#define FIZZ_DECLARE_COPYABLE_VARIANT_TYPE(NAME, X)                \
  struct NAME {                                                    \
    enum class Type { X(FIZZ_ENUM_TYPES) };                        \
                                                                   \
    X(FIZZ_UNION_CTORS, NAME)                                      \
                                                                   \
    X(FIZZ_UNION_COPY_CTORS, NAME)                                 \
                                                                   \
    NAME(NAME&& other) {                                           \
      switch (other.type_) { X(FIZZ_UNION_MOVE_CASES, other) }     \
      type_ = other.type_;                                         \
    }                                                              \
                                                                   \
    NAME& operator=(NAME&& other) {                                \
      destroyVariant();                                            \
      switch (other.type_) { X(FIZZ_UNION_MOVE_CASES, other) }     \
      type_ = other.type_;                                         \
      return *this;                                                \
    }                                                              \
                                                                   \
    NAME(const NAME& other) {                                      \
      switch (other.type_) { X(FIZZ_UNION_COPY_CASES, other) }     \
      type_ = other.type_;                                         \
    }                                                              \
                                                                   \
    NAME& operator=(const NAME& other) {                           \
      destroyVariant();                                            \
      switch (other.type_) { X(FIZZ_UNION_COPY_CASES, other) }     \
      type_ = other.type_;                                         \
      return *this;                                                \
    }                                                              \
                                                                   \
    bool operator==(const NAME& other) const {                     \
      if (other.type() != type_) {                                 \
        return false;                                              \
      }                                                            \
      switch (other.type_) { X(FIZZ_UNION_EQUALITY_CASES, other) } \
      return false;                                                \
    }                                                              \
                                                                   \
    ~NAME() {                                                      \
      destroyVariant();                                            \
    }                                                              \
                                                                   \
    Type type() const {                                            \
      return type_;                                                \
    }                                                              \
                                                                   \
    X(FIZZ_UNION_ACCESSOR)                                         \
                                                                   \
    X(FIZZ_CONST_UNION_ACCESSOR)                                   \
                                                                   \
    template <class T, class E>                                    \
    struct TypedGet {};                                            \
    X(FIZZ_UNION_TYPED_GET)                                        \
                                                                   \
    /* Do not use in non-test code */                              \
    template <class T>                                             \
    T* getType() {                                                 \
      return TypedGet<T, int>::get(*this);                         \
    }                                                              \
    template <class T>                                             \
    const T* getType() const {                                     \
      return TypedGet<T, int>::getConst(*this);                    \
    }                                                              \
                                                                   \
   private:                                                        \
    union {                                                        \
      X(FIZZ_UNION_TYPE)                                           \
    };                                                             \
                                                                   \
    void destroyVariant() {                                        \
      switch (type_) { X(FIZZ_DESTRUCTOR_CASES) }                  \
    }                                                              \
                                                                   \
    Type type_;                                                    \
  };
} // namespace fizz
