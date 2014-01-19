/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef HPHP_THRIFT_THRIFT_H
#define HPHP_THRIFT_THRIFT_H

#include "thrift/lib/cpp/thrift_config.h"

#include <assert.h>
#include <sys/types.h>
#ifdef HPHP_THRIFT_HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <exception>
#include <iterator>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <typeinfo>
#include <vector>

#include "thrift/lib/cpp/TLogging.h"

namespace apache { namespace thrift {

struct ltstr {
  bool operator()(const char* s1, const char* s2) const {
    return strcmp(s1, s2) < 0;
  }
};

/**
 * Marker class, for enabling CRTP-based helper functions. All user-defined
 * struct types will inherit from this type, so template functions can match any
 * of these types by using 'TStructType<ActualType>' in their signature.
 *
 * Concrete types can be obtained through __self() member function.
 */
template<typename Self>
struct TStructType {
  const Self& __self() const {
    return static_cast<const Self&>(*this);
  }

  Self& __self() {
    return static_cast<Self&>(*this);
  }
};

/**
 * Helper template class for enum<->string conversion.
 */
template<typename T>
struct TEnumTraitsBase {
  /**
   * Finds the name of a given enum value, returning it or nullptr on failure.
   * Specialized implementations will be emitted as part of enum codegen.
   *
   * Example specialization:
   *   template<>
   *   const char* TEnumTraits<MyEnum>::findName(MyEnum value) {
   *     return findName(_MyEnum_VALUES_TO_NAMES, value);
   *   }
   * Note the use of helper function 'findName(...)', below.
   */
  static const char* findName(T value);
  /**
   * Attempts to find a value for a given name.
   * Specialized implementations will be emitted as part of enum codegen.
   *
   * Example implementation:
   *   template<>
   *   bool TEnumTraits<MyEnum>::findValue(const char* name,
   *                                        MyEnum* outValue) {
   *     return findValue(_MyEnum_NAMES_TO_VALUES, name, outValue);
   *   }
   * Note the use of helper function 'findValue(...)', below.
   */
  static bool findValue(const char* name, T* outValue);
 private:
  /**
   * Helper method used by codegen implementation of findName, Supports
   * use with strict and non-strict enums by way of template parameter
   * 'ValueType'.
   */
  template<typename ValueType>
  static const char* findName(const std::map<ValueType, const char*>& map,
                              T value) {
    auto found = map.find(value);
    if (found == map.end()) {
      return nullptr;
    } else {
      return found->second;
    }
  }

  /**
   * Helper method used by codegen implementation of findValue, Supports
   * use with strict and non-strict enums by way of template parameter
   * 'ValueType'.
   */
  template<typename ValueType>
  static bool findValue(const std::map<const char*, ValueType, ltstr>& map,
                        const char* name, T* out) {
    auto found = map.find(name);
    if (found == map.end()) {
      return false;
    } else {
      *out = static_cast<T>(found->second);
      return true;
    }
  }
};

template<typename T>
struct TEnumTraits : public TEnumTraitsBase<T> {
  /**
   * Return the minimum value.
   */
  static constexpr T min();

  /**
   * Return the maximum value.
   */
  static constexpr T max();
};

template <typename T>
class TEnumIterator : public std::map<T, char*>::iterator {
 public:
  TEnumIterator(int n,
                T* enums,
                const char** names) :
      ii_(0), n_(n), enums_(enums), names_(names) {
  }

  int operator ++() {
    return ++ii_;
  }

  bool operator !=(const TEnumIterator<T>& end) {
    assert(end.n_ == -1);
    return (ii_ != n_);
  }

  std::pair<T, const char*> operator*() const {
    return std::make_pair(enums_[ii_], names_[ii_]);
  }

 private:
  int ii_;
  const int n_;
  T* enums_;
  const char** names_;
};

template <typename T>
class TEnumInverseIterator : public std::map<T, char*>::iterator {
 public:
  TEnumInverseIterator(int n,
                       T* enums,
                       const char** names) :
      ii_(0), n_(n), enums_(enums), names_(names) {
  }

  int operator ++() {
    return ++ii_;
  }

  bool operator !=(const TEnumInverseIterator<T>& end) {
    assert(end.n_ == -1);
    return (ii_ != n_);
  }

  std::pair<const char*, T> operator*() const {
    return std::make_pair(names_[ii_], enums_[ii_]);
  }

 private:
  int ii_;
  const int n_;
  T* enums_;
  const char** names_;
};

class TOutput {
 public:
  TOutput() : f_(&errorTimeWrapper) {}

  inline void setOutputFunction(void (*function)(const char *)){
    f_ = function;
  }

  inline void operator()(const char *message){
    f_(message);
  }

  // It is important to have a const char* overload here instead of
  // just the string version, otherwise errno could be corrupted
  // if there is some problem allocating memory when constructing
  // the string.
  void perror(const char *message, int errno_copy);
  inline void perror(const std::string &message, int errno_copy) {
    perror(message.c_str(), errno_copy);
  }

  void printf(const char *message, ...);

  inline static void errorTimeWrapper(const char* msg) {
    time_t now;
    char dbgtime[26];
    time(&now);
    ctime_r(&now, dbgtime);
    dbgtime[24] = 0;
    fprintf(stderr, "Thrift: %s %s\n", dbgtime, msg);
  }

  /** Just like strerror_r but returns a C++ string object. */
  static std::string strerror_s(int errno_copy);

 private:
  void (*f_)(const char *);
};

extern TOutput GlobalOutput;

/**
 * Base class for all Thrift exceptions.
 * Should never be instantiated, only caught.
 */
class TException : public std::exception {
public:
  TException() {}
  TException(TException&&) {}
  TException(const TException&) {}
  TException& operator=(const TException&) { return *this; }
  TException& operator=(TException&&) { return *this; }
};

/**
 * Marker class, for enabling CRTP-based helper functions. All user-defined
 * exception types will inherit from this type, so template functions can match
 * any of these types by using 'TExceptionType<ActualType>' in their signature.
 *
 * Concrete types can be obtained through __self() member function.
 */
template<class Self>
struct TExceptionType : TException {
  const Self& __self() const {
    return static_cast<const Self&>(*this);
  }

  Self& __self() {
    return static_cast<Self&>(*this);
  }
};

/**
 * Base class for exceptions from the Thrift library, and occasionally
 * from the generated code.  This class should not be thrown by user code.
 * Instances of this class are not meant to be serialized.
 */
class TLibraryException : public TException {
 public:
  TLibraryException() {}

  explicit TLibraryException(const std::string& message) :
    message_(message) {}

  TLibraryException(const char* message, int errnoValue);

  virtual ~TLibraryException() throw() {}

  virtual const char* what() const throw() {
    if (message_.empty()) {
      return "Default TLibraryException.";
    } else {
      return message_.c_str();
    }
  }

 protected:
  std::string message_;

};

#if T_GLOBAL_DEBUG_VIRTUAL > 1
void profile_virtual_call(const std::type_info& info);
void profile_generic_protocol(const std::type_info& template_type,
                              const std::type_info& prot_type);
void profile_print_info(FILE *f);
void profile_print_info();
void profile_write_pprof(FILE* gen_calls_f, FILE* virtual_calls_f);
#endif

template <class ThriftContainer>
inline void reallyClear(ThriftContainer& container) {
  ThriftContainer emptyContainer;
  swap(container, emptyContainer);
}

template <typename T>
inline void merge(const T& from, T& to) {
  to = from;
}

template <typename T>
inline void merge(T&& from, T& to) {
  to = std::move(from);
}

template <typename T, typename A>
inline void merge(const std::vector<T, A>& from, std::vector<T, A>& to) {
  std::copy(from.begin(), from.end(), std::back_inserter(to));
}

template <typename T, typename A>
inline void merge(std::vector<T, A>&& from, std::vector<T, A>& to) {
  std::move(from.begin(), from.end(), std::back_inserter(to));
}

template <typename K, typename C, typename A>
inline void merge(const std::set<K, C, A>& from, std::set<K, C, A>& to) {
  std::copy(from.begin(), from.end(), std::inserter(to, to.end()));
}

template <typename K, typename C, typename A>
inline void merge(std::set<K, C, A>&& from, std::set<K, C, A>& to) {
  std::move(from.begin(), from.end(), std::inserter(to, to.end()));
}

template <typename K, typename T, typename C, typename A>
void merge(const std::map<K, T, C, A>& from, std::map<K, T, C, A>& to) {
  for (auto& kv : from) {
    merge(kv.second, to[kv.first]);
  }
}

template <typename K, typename T, typename C, typename A>
void merge(std::map<K, T, C, A>&& from, std::map<K, T, C, A>& to) {
  for (auto&& kv : from) {
    merge(std::move(kv.second), to[kv.first]);
  }
}

}} // apache::thrift

#endif // #ifndef HPHP_THRIFT_THRIFT_H
