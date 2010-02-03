/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_JSON_H__
#define __HPHP_JSON_H__

#include "base.h"

namespace HPHP { namespace JSON {
///////////////////////////////////////////////////////////////////////////////

class OutputStream;
class MapStream;

std::string Escape(const char *s);

class ISerializable {
public:
  virtual ~ISerializable() {}

  /**
   * Generate JSON output of this data structure.
   */
  virtual void serialize(OutputStream &out) const = 0;
};

class Name : public ISerializable {
public:
  Name(const char *name);
  Name(const std::string &name);

  // implement ISerializable
  virtual void serialize(OutputStream &out) const;

private:
  std::string m_name;
};

class OutputStream {
public:
  OutputStream(std::ostream &out) : m_out(out) {}

  OutputStream &operator<< (int v);
  OutputStream &operator<< (const char *v);
  OutputStream &operator<< (const std::string &v);
  OutputStream &operator<< (const Name &v);
  OutputStream &operator<< (const ISerializable &v);

  template<typename T>
    OutputStream &operator<< (const boost::shared_ptr<T> &v);

  template<typename T>
    OutputStream &operator<< (const std::vector<T> &v);
  template<typename T>
    OutputStream &operator<< (const std::set<T> &v);
  template<typename K, typename T, typename C>
    OutputStream &operator<< (const std::map<K, T, C> &v);

  template<typename K, typename T, typename C>
    OutputStream &operator<< (const __gnu_cxx::hash_map<K, T, C> &v);

  std::ostream &raw() { return m_out;}

private:
  std::ostream &m_out;
};

template<typename T>
  OutputStream &OutputStream::operator<< (const boost::shared_ptr<T> &v) {
  if (v) {
    *this << *v;
  } else {
    m_out << "null";
  }
  return *this;
}

template<typename T>
  OutputStream &OutputStream::operator<< (const std::vector<T> &v) {
  m_out << "[";
  for (unsigned int i = 0; i < v.size(); i++) {
    if (i > 0) m_out << ',';
    *this << v[i];
  }
  m_out << "]";
  return *this;
}

template<typename T>
  OutputStream &OutputStream::operator<< (const std::set<T> &v) {
  m_out << "[";
  bool first = true;
  BOOST_FOREACH(T el, v) {
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

template<typename K, typename T, typename C>
  OutputStream &OutputStream::operator<< (const std::map<K, T, C> &v) {
  m_out << "{";
  for (typename std::map<K, T, C>::const_iterator iter = v.begin();
       iter != v.end(); ++iter) {
    if (iter != v.begin()) m_out << ',';
    *this << Name(iter->first);
    *this << iter->second;
  }
  m_out << "}\n";
  return *this;
}

template<typename K, typename T, typename C>
  OutputStream &OutputStream::operator<<
  (const __gnu_cxx::hash_map<K, T, C> &v) {
  m_out << "{";
  for (typename __gnu_cxx::hash_map<K, T, C>::const_iterator
         iter = v.begin(); iter != v.end(); ++iter) {
    if (iter != v.begin()) m_out << ',';
    *this << Name(iter->first);
    *this << iter->second;
  }
  m_out << "}\n";
  return *this;
}

class MapStream {
public:
  MapStream(OutputStream &jout)
    : m_out(jout.raw()), m_jout(jout), m_first(true) {}
  /*
  MapStream &add(const std::string &n, int v);
  MapStream &add(const std::string &n, const char *v);
  MapStream &add(const std::string &n, const std::string &v);
  MapStream &add(const std::string &n, const ISerializable &v);

  template<typename K, typename T, typename C>
    MapStream &add(const std::string &n, const std::map<K, T, C> &v);

  template<typename T>
    MapStream &add(const std::string &n, const std::vector<T> &v);
  */

  template<typename T>
    MapStream &add(const std::string &n, T v);

  void done() {
    if (m_first) {
      m_out << "{";
    }
    m_out << "}\n";
  }

private:
  std::ostream &m_out;
  OutputStream &m_jout;
  bool m_first;

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

template<typename T>
  MapStream &MapStream::add(const std::string &n, T v) {
  init(n);
  m_jout << v;
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_JSON_H__
