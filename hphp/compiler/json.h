/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JSON_H_
#define incl_HPHP_JSON_H_

#include <cassert>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <vector>

#include "hphp/util/hash-map-typedefs.h"

namespace HPHP {

struct AnalysisResult;

namespace JSON {
///////////////////////////////////////////////////////////////////////////////

template <typename T> class _OutputStream;
template <typename T> class _MapStream;
template <typename T> class _ListStream;
template <typename T> class _ISerializable;

#define DEFINE_JSON_OUTPUT_TYPE(type) \
  struct type { \
    typedef _OutputStream<type>  OutputStream; \
    typedef _MapStream<type>     MapStream; \
    typedef _ListStream<type>    ListStream; \
    typedef _ISerializable<type> ISerializable; \
  }

DEFINE_JSON_OUTPUT_TYPE(CodeError);
DEFINE_JSON_OUTPUT_TYPE(DocTarget);

std::string Escape(const char *s);

template <typename T>
class _ISerializable {
public:
  virtual ~_ISerializable() {}

  /**
   * Generate JSON output of this data structure.
   */
  virtual void serialize(_OutputStream<T> &out) const = 0;
};

class Name {
public:
  explicit Name(const char *name) {
    assert(name && *name);
    m_name = name;
  }
  explicit Name(const std::string &name) {
    assert(!name.empty());
    m_name = name;
  }
  const std::string &getName() const { return m_name; }
private:
  std::string m_name;
};

// struct _Null {};
// _Null Null;
enum class Null {};

template <typename Type>
class _OutputStream {
public:
  _OutputStream(std::ostream &out,
                std::shared_ptr<AnalysisResult> ar) : m_out(out), m_ar(ar) {}

  _OutputStream &operator<< (unsigned int v) { m_out << v; return *this; }

  _OutputStream &operator<< (int v) { m_out << v; return *this; }

  _OutputStream &operator<< (bool v) {
    m_out << (v ? "true" : "false");
    return *this;
  }

  _OutputStream &operator<< (const char *v) {
    m_out << "\"" << Escape(v) << "\"";
    return *this;
  }

  _OutputStream &operator<< (const std::string &v) {
    m_out << "\"" << Escape(v.c_str()) << "\"";
    return *this;
  }

  _OutputStream &operator<< (const Name &n) {
    m_out << "\"" << n.getName() << "\":";
    return *this;
  }

  _OutputStream &operator<< (const Null &n) {
    m_out << "null";
    return *this;
  }

  _OutputStream &operator<< (const _ISerializable<Type> &v) {
    v.serialize(*this);
    return *this;
  }

  template<typename T>
  _OutputStream &operator<< (const std::shared_ptr<T> &v) {
    if (v) {
      *this << *v;
    } else {
      *this << Null();
    }
    return *this;
  }

  template<typename T>
  _OutputStream &operator<< (const std::vector<T> &v) {
    m_out << "[";
    for (unsigned int i = 0; i < v.size(); i++) {
      if (i > 0) m_out << ',';
      *this << v[i];
    }
    m_out << "]";
    return *this;
  }

  template<typename T>
  _OutputStream &operator<< (const std::set<T> &v) {
    m_out << "[";
    bool first = true;
    for (T el: v) {
      if (first) {
        first = false;
      } else {
        m_out << ',';
      }
      *this << el;
    }
    m_out << "]";
    return *this;
  }

  // TODO: std::map and __gnu_cxx::hash_map should share
  // the same function...

  template<typename K, typename T, typename C>
  _OutputStream &operator<< (const std::map<K, T, C> &v) {
    m_out << "{";
    for (auto iter = v.begin(); iter != v.end(); ++iter) {
      if (iter != v.begin()) m_out << ',';
      *this << Name(iter->first);
      *this << iter->second;
    }
    m_out << "}\n";
    return *this;
  }

  template<typename K, typename T, typename H, typename E>
  _OutputStream &operator<< (const hphp_hash_map<K, T, H, E> &v) {
    m_out << "{";
    for (auto iter = v.begin(); iter != v.end(); ++iter) {
      if (iter != v.begin()) m_out << ',';
      *this << Name(iter->first);
      *this << iter->second;
    }
    m_out << "}\n";
    return *this;
  }

  std::shared_ptr<AnalysisResult> analysisResult() const { return m_ar; }

private:
  std::ostream      &m_out;
  std::shared_ptr<AnalysisResult>  m_ar;

  std::ostream &raw() { return m_out;}

  friend class _MapStream<Type>;
  friend class _ListStream<Type>;
};

template <typename Type>
class _MapStream {
public:
  explicit _MapStream(_OutputStream<Type> &jout)
    : m_out(jout.raw()), m_jout(jout), m_first(true) {}

  template<typename T>
  _MapStream &add(const std::string &n, T v) {
    init(n);
    m_jout << v;
    return *this;
  }

  _MapStream &add(const std::string &n) {
    init(n);
    return *this;
  }

  void done() {
    if (m_first) {
      m_out << "{";
    }
    m_out << "}\n";
  }

private:
  std::ostream        &m_out;
  _OutputStream<Type> &m_jout;
  bool                m_first;

  void init(const std::string &n) {
    if (m_first) {
      m_out << "{";
      m_first = false;
    } else {
      m_out << ",";
    }
    m_jout << Name(n);
  }
};

template <typename Type>
class _ListStream {
public:
  explicit _ListStream(_OutputStream<Type> &jout)
    : m_out(jout.raw()), m_jout(jout), m_first(true) {}

  void next() {
    if (m_first) {
      m_out << "[";
      m_first = false;
    } else {
      m_out << ",";
    }
  }

  template<typename T>
  _ListStream &operator<< (T &v) {
    next();
    m_jout << v;
    return *this;
  }

  void done() {
    if (m_first) {
      m_out << "[";
    }
    m_out << "]\n";
  }

private:
  std::ostream        &m_out;
  _OutputStream<Type> &m_jout;
  bool                m_first;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_JSON_H_
