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

#ifndef __HPHP_HPHP_MAP_H__
#define __HPHP_HPHP_MAP_H__

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/util/hphp_vector.h>
#include <runtime/base/util/hphp_map_cell.h>
#include <runtime/base/zend/zend_string.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#ifndef OLD_HPHP_MAP

class HphpMap;
class HphpMapConstIterator;
class HphpMapIterator;

class HphpMapConstIterator {
  typedef HphpMap Map;
  typedef HphpMapCell Cell;
  uint m_idx;
  const Map *m_map;
 public:
  HphpMapConstIterator(uint idx, const Map *map)
    : m_idx(idx), m_map(map) {}

  HphpMapConstIterator() : m_idx(0), m_map(NULL) {}
  HphpMapConstIterator(const HphpMapConstIterator& it)
    : m_idx(it.m_idx), m_map(it.m_map) {}
  HphpMapConstIterator(const HphpMapIterator& it);

  const Cell& operator*() const {
    return get();
  }

  const Cell* operator->() const {
    return &get();
  }
  HphpMapConstIterator& operator++();
  bool operator==(const HphpMapConstIterator& it) const {
    return m_idx == it.m_idx;
  }
  bool operator!=(const HphpMapConstIterator& it) const {
    return m_idx != it.m_idx;
  }
private:
  const Cell& get() const;
};

class HphpMapIterator {
  typedef HphpMap Map;
  typedef HphpMapCell Cell;
  uint m_idx;
  Map *m_map;
 public:
  HphpMapIterator(uint idx, Map *map)
    : m_idx(idx), m_map(map) {}

  HphpMapIterator() : m_idx(0), m_map(NULL) {}
  HphpMapIterator(const HphpMapIterator& it)
    : m_idx(it.m_idx), m_map(it.m_map) {}

  Cell& operator*() {
    return get();
  }

  Cell* operator->() {
    return &get();
  }
  uint getidx() const {
    return m_idx;
  }
  const Map* getmap() const {
    return m_map;
  }

  HphpMapIterator& operator++();
  bool operator==(const HphpMapIterator& it) const {
    return m_idx == it.m_idx;
  }
  bool operator!=(const HphpMapIterator& it) const {
    return m_idx != it.m_idx;
  }
private:
  Cell& get();
};

class HphpMap {
  friend class HphpMapConstIterator;
  friend class HphpMapIterator;
  typedef HphpMapCell Cell;
 public:
  HphpMap() : m_table(32), m_size(0), m_useSize(0) {
  }
  HphpMap(const HphpMap& map) {
    copy(map);
  }
  ~HphpMap() { clear(); }

  bool insert(CVarRef key, int value, int &oldValue, int64 preHash = -1) {
    ASSERT(preHash < 0 || preHash == hash(key));

    int64 idx = preHash >= 0 ? preHash : hash(key);
    int64 hash = idx;
    uint mask = m_table.size() - 1;
    ASSERT((mask & (mask+1)) == 0);
    while (true) {
      idx &= mask;
      Cell &cell = m_table[idx];
      if (cell.empty()) {
        cell.set(key, hash, value);
        oldValue = value;
        break;
      }
      if (cell.hash() == hash && same(key, cell)) {
        oldValue = cell.value();
        return false;
      }
      idx++;
    }
    m_size++;
    m_useSize++;
    if (overloaded()) {
      growTable();
    }
    return true;
  }

  int& operator[](CVarRef key);
  HphpMap& operator=(const HphpMap& map) {
    copy(map);
    return *this;
  }
  void swap(HphpMap& map) {
    m_table.swap(map.m_table);
    uint s = m_size;
    m_size = map.m_size;
    map.m_size = s;

    s = m_useSize;
    m_useSize = map.m_useSize;
    map.m_useSize = s;
  }

  bool find(CVarRef key, int &value, int64 preHash = -1) const {
    ASSERT(preHash < 0 || preHash == hash(key));

    int64 idx = preHash >= 0 ? preHash : hash(key);
    int64 hash = idx;
    uint mask = m_table.size() - 1;
    ASSERT((mask & (mask+1)) == 0);
    while (true) {
      idx &= mask;
      const Cell &cell = m_table[idx];
      if (cell.empty()) {
        return false;
      }
      if (cell.hash() == hash && same(key, cell)) {
        value = cell.value();
        return true;
      }
      idx++;
    }
  }

  const Cell &findCell(CVarRef key) const {
    int64 idx = hash(key);
    int64 hash = idx;
    uint mask = m_table.size() - 1;
    ASSERT((mask & (mask+1)) == 0);
    while (true) {
      idx &= mask;
      const Cell &cell = m_table[idx];
      if (cell.empty()) {
        ASSERT(false);
        return cell;
      }
      if (cell.hash() == hash && same(key, cell)) {
        return cell;
      }
      idx++;
    }
  }
  const int rawFind(int64 hash, int64 num) const {
    ASSERT(hash != -1);
    int64 idx = hash;
    uint mask = m_table.size() - 1;
    ASSERT((mask & (mask+1)) == 0);
    while (true) {
      idx &= mask;
      const Cell &cell = m_table[idx];
      if (cell.empty()) {
        return -1;
      }
      if (cell.hash() == hash && num == cell.num()) {
        return cell.value();
      }
      idx++;
    }
  }


  void erase(CVarRef key);
  void erase(CVarRef key, int64 preHash);
  uint size() const { return m_size; }

  typedef HphpMapConstIterator const_iterator;
  typedef HphpMapIterator iterator;
  const_iterator begin() const {
    return HphpMapConstIterator(firstIdx(), this);
  }
  const_iterator end() const {
    return HphpMapConstIterator(m_table.size(), this);
  }
  iterator begin() {
    return HphpMapIterator(firstIdx(), this);
  }
  iterator end() {
    return HphpMapIterator(m_table.size(), this);
  }

  uint firstIdx() const {
    for (uint idx = 0; idx < m_table.size(); idx++) {
      if (m_table[idx].alive())
        return idx;
    }
    return m_table.size();
  }
  void copy(const HphpMap& map) {
    ASSERT(&map != this);

    m_table.resize(map.m_table.size());
    for (const_iterator it = map.begin(); it != map.end(); ++it) {
      insertCell(*it, m_table, true);
    }
    m_size = m_useSize = map.m_size;
  }
  void clear() {
    uint size = m_table.size();
    for (uint i = 0; i < size; i++) {
      Cell& cell = m_table[i];
      if (cell.type() == KindOfString) cell.clear();
    }
    m_size = m_useSize = 0;
  }

  static int64 hash(CVarRef s) {
    int64 hash;
    switch (s.getType()) {
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
      hash = hash_int64(s.getNumData());
      break;
    case LiteralString:
      {
        const char* d = s.getLiteralString();
        hash = hash_string(d, strlen(d));
      }
      break;
    case KindOfStaticString:
    case KindOfString:
      {
        StringData *st = s.getStringData();
        hash = hash_string(st->data(), st->size());
      }
      break;
    default:
      ASSERT(false);
      return 0;
    }
    return hash;
  }
  static bool same(CVarRef s1, const Cell &s2) {
    DataType t1 = s1.getType();
    DataType t2 = s2.type();
    switch (t1) {
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
      switch (t2) {
      case KindOfInt16:
      case KindOfInt32:
      case KindOfInt64:
        break;
      default:
        return false;
      }
      break;
    case LiteralString:
    case KindOfStaticString:
    case KindOfString:
      switch (t2) {
      case LiteralString:
      case KindOfStaticString:
      case KindOfString:
        break;
      default:
        return false;
      }
      break;
    default:
      ASSERT(false);
      if (t1 != t2) return false;
      break;
    }

    switch (t1) {
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
      return s1.getNumData() == s2.toInt64();
    default:
      const char* s1d;
      uint64 s1l;
      const char* s2d;
      uint64 s2l;
      if (t1 == LiteralString) {
        s1d = s1.getLiteralString();
        s1l = strlen(s1d);
      } else {
        StringData *s1data = s1.getStringData();
        s1d = s1data->data();
        s1l = s1data->size();
      }
      if (t2 == LiteralString) {
        s2d = s2.getLiteralString();
        s2l = strlen(s2d);
      } else {
        StringData *s2data = s2.getStringData();
        s2d = s2data->data();
        s2l = s2data->size();
      }
      return (s1d == s2d && s1l == s2l) ||
        string_strcmp(s1d, s1l, s2d, s2l) == 0;
    }
  }

  /**
   * Memory allocator methods.
   */
  bool calculate(int &size) {
    return m_table.calculate(size);
  }
  void backup(LinearAllocator &allocator) {
    m_table.backup(allocator);
  }
  void restore(const char *&data) {
    m_table.restore(data);
  }
  void sweep() {
    m_table.sweep();
  }

  void dump();

 protected:
  HphpVector<Cell> m_table;
  uint m_size;
  uint m_useSize;

 private:
  void growTable();
  void insertCell(const Cell &cell, HphpVector<Cell> &table, bool copy = false);
  bool overloaded() {
    uint tsize = m_table.size();
    return m_useSize > (tsize >> 1);
  }
};

#else
//Old HphpMap

class HphpMap;
class HphpMapConstIterator;
class HphpMapIterator;

class HphpMapNode {
  typedef HphpMapNode Node;

  friend class HphpMap;
  friend class HphpMapConstIterator;
  friend class HphpMapIterator;
 public:
  DECLARE_SMART_ALLOCATION_NOCALLBACKS(HphpMapNode);
  HphpMapNode(CVarRef k, int v, HphpMapNode* next)
    : m_value(v), m_key(k), m_next(next) {}


  CVarRef key() const { return m_key; }

  void dump();
  const int &value() const { return m_value; }
  int &lvalue() { return m_value; }
 private:
  int m_value;
  Variant m_key;
  HphpMapNode* m_next;
};

class HphpMapConstIterator {
  typedef HphpMap Map;
  typedef HphpMapNode Node;
  const Node *m_node;
  const Map *m_map;
 public:
  HphpMapConstIterator(const Node *node, const Map *map)
    : m_node(node), m_map(map) {}

  HphpMapConstIterator() : m_node(NULL), m_map(NULL) {}
  HphpMapConstIterator(const HphpMapConstIterator& it)
    : m_node(it.m_node), m_map(it.m_map) {}
  HphpMapConstIterator(const HphpMapIterator& it);

  const Node& operator*() const {
    return *m_node;
  }

  const Node* operator->() const {
    return m_node;
  }
  HphpMapConstIterator& operator++();
  bool operator==(const HphpMapConstIterator& it) const {
    return m_node == it.m_node;
  }
  bool operator!=(const HphpMapConstIterator& it) const {
    return m_node != it.m_node;
  }
};

class HphpMapIterator {
  typedef HphpMap Map;
  typedef HphpMapNode Node;
  Node *m_node;
  const Map *m_map;
 public:
  HphpMapIterator(Node *node, const Map *map)
    : m_node(node), m_map(map) {}

  HphpMapIterator() : m_node(NULL), m_map(NULL) {}
  HphpMapIterator(const HphpMapIterator& it)
    : m_node(it.m_node), m_map(it.m_map) {}

  Node& operator*() const {
    return *m_node;
  }

  Node* operator->() const {
    return m_node;
  }

  Node* get() const {
    return m_node;
  }
  const Map* getmap() const {
    return m_map;
  }

  HphpMapIterator& operator++();
  bool operator==(const HphpMapIterator& it) const {
    return m_node == it.m_node;
  }
  bool operator!=(const HphpMapIterator& it) const {
    return m_node != it.m_node;
  }
};

class HphpMap {
  friend class HphpMapConstIterator;
  friend class HphpMapIterator;
  typedef HphpMapNode Node;
 public:
  HphpMap() : m_table(23), m_size(0) {
  }
  HphpMap(const HphpMap& map) {
    copy(map);
  }
  ~HphpMap() { clear(); }

  bool insert(CVarRef key, int value, int &oldValue, int64 preHash = -1);

  int& operator[](CVarRef key);
  HphpMap& operator=(const HphpMap& map) {
    copy(map);
    return *this;
  }
  void swap(HphpMap& map) {
    m_table.swap(map.m_table);
    size_t s = m_size;
    m_size = map.m_size;
    map.m_size = s;
  }

  bool find(CVarRef key, int &value, int64 preHash = -1) const;
  void erase(CVarRef key);
  void erase(CVarRef key, int64 preHash);
  size_t size() const { return m_size; }

  typedef HphpMapConstIterator const_iterator;
  typedef HphpMapIterator iterator;
  const_iterator begin() const {
    return HphpMapConstIterator(firstNode(), this);
  }
  const_iterator end() const {
    return HphpMapConstIterator(NULL, this);
  }
  iterator begin() {
    return HphpMapIterator(firstNode(), this);
  }
  iterator end() {
    return HphpMapIterator(NULL, this);
  }

  Node* firstNode() const {
    for (uint idx = 0; idx < m_table.size(); idx++) {
      Node* node = m_table[idx];
      if (node) {
        return node;
      }
    }
    return NULL;
  }
  void clear();
  void copy(const HphpMap& map);

  static int64 hash(CVarRef str);
  static bool same(CVarRef s1, CVarRef s2);

  /**
   * Memory allocator methods.
   */
  bool calculate(int &size) {
    return m_table.calculate(size);
  }
  void backup(LinearAllocator &allocator) {
    m_table.backup(allocator);
  }
  void restore(const char *&data) {
    m_table.restore(data);
  }
  void sweep() {
    m_table.sweep();
  }
  void dump() {}

 protected:
  HphpVector<Node*> m_table;
  size_t m_size;

 private:
  void growTable();
  void insertNode(Node* node, HphpVector<Node*> &table);
  bool overloaded() {
    size_t tsize = m_table.size();
    return m_size > tsize - (tsize >> 2);
  }
};

#endif

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_HPHP_MAP_H__
