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

#ifndef __HPHP_HPHP_MAP_CELL_H__
#define __HPHP_HPHP_MAP_CELL_H__

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
#ifndef OLD_HPHP_MAP

class HphpMapCell {
public:
 HphpMapCell() : m_hash(0), m_value(0), m_type(KindOfNull) {}

  Variant key() const {
    switch (m_type) {
    case KindOfString:
      return Variant(m_data.pstr);
    case KindOfInt64:
      return Variant(m_data.num);
    case LiteralString:
      return Variant(m_data.str);
    default:
      ASSERT(false);
      return Variant();
    }
  }
  void dump();
  void clear() {
    if (m_type == KindOfString) {
      if (m_data.pstr->decRefCount() == 0) {
        m_data.pstr->release();
      }
    }
    m_data.num = 0;
    m_hash = 0;
    m_value = 0;
    m_type = KindOfNull;
  }

  int64 toInt64() const {
    ASSERT(m_type == KindOfInt64);
    return m_data.num;
  }
  StringData *getStringData() const {
    ASSERT(m_type == KindOfString);
    return m_data.pstr;
  }
  litstr getLiteralString() const {
    ASSERT(m_type == LiteralString);
    return m_data.str;
  }

  void set(CVarRef key, int64 h, int v) {
    m_type = key.getType();
    switch (m_type) {
    case KindOfString:
      m_data.pstr = key.getStringData();
      m_data.pstr->incRefCount();
      break;
    case LiteralString:
      m_data.str = key.getLiteralString();
      break;
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
      m_type = KindOfInt64;
      m_data.num = key.toInt64();
      break;
    default:
      ASSERT(false);
    }
    m_hash = h;
    m_value = v;
  }

  void set(const HphpMapCell &c, bool copy) {
    m_value = c.m_value;
    m_hash = c.m_hash;
    m_data.num = c.m_data.num;
    m_type = c.m_type;
    if (copy && m_type == KindOfString) {
      m_data.pstr->incRefCount();
    }
  }

  bool empty() const {
    return m_type == KindOfNull && m_value == 0;
  }
  bool alive() const {
    return m_type != KindOfNull;
  }
  bool deleted() const {
    return m_type == KindOfNull && m_value == -1;
  }
  void erase() {
    clear();
    m_value = -1;
  }

  int64 hash() const { return m_hash; }
  int64 num() const { return m_data.num; }
  const int &value() const { return m_value; }
  int &lvalue() { return m_value; }
  DataType type() const { return m_type; }

private:
  HphpMapCell(const HphpMapCell &cell) {}

  int64 m_hash;
  union {
    int64 num;
    StringData *pstr;
    litstr str;
  } m_data;
  int m_value;
  DataType m_type;

};

namespace HphpVectorFuncs {
// HphpVector<HphpMapCell>
inline void allocate(HphpMapCell *data, int count) {
}
inline void deallocate(HphpMapCell *data, int count) {
}
inline void reset(HphpMapCell *data, int count) {}
inline void sweep(HphpMapCell *data, int count) {}
inline void copy(HphpMapCell *dest, HphpMapCell *src, int count) {
  for (int i = 0; i < count; i++) {
    dest[i] = src[i];
  }
}

}

#endif

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_HPHP_MAP_CELL_H__ */
