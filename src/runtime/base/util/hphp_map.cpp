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

#include <runtime/base/util/hphp_map.h>
#include <runtime/base/zend/zend_functions.h>
#include <runtime/base/complex_types.h>

using namespace HPHP;
///////////////////////////////////////////////////////////////////////////////
#ifndef OLD_HPHP_MAP
void HphpMapCell::dump() {
  ASSERT(alive());
  key().dump();
  printf("value: %d\n", m_value);
}

HphpMapConstIterator::HphpMapConstIterator(const HphpMapIterator& it)
  : m_idx(it.getidx()), m_map(it.getmap()) {
}

const HphpMapCell &HphpMapConstIterator::get() const {
  return m_map->m_table[m_idx];
}

HphpMapConstIterator& HphpMapConstIterator::operator++() {
  const HphpVector<Cell> &table = m_map->m_table;
  ssize_t tsize = table.size();
  if (m_idx != tsize) {
    m_idx++;
    while (m_idx < tsize && !table[m_idx].alive()) { m_idx++; }
  }
  return *this;
}

HphpMapCell &HphpMapIterator::get() {
  return m_map->m_table[m_idx];
}

HphpMapIterator& HphpMapIterator::operator++() {
  const HphpVector<Cell> &table = m_map->m_table;
  ssize_t tsize = table.size();
  if (m_idx != tsize) {
    m_idx++;
    while (m_idx < tsize && !table[m_idx].alive()) { m_idx++; }
  }
  return *this;
}

int& HphpMap::operator[](CVarRef key) {
  int64 idx = hash(key);
  int64 hash = idx;
  Cell* cell;
  while (true) {
    idx &= m_table.size() - 1;
    cell = &m_table[idx];
    if (cell->empty()) {
      break;
    }
    if (cell->hash() == hash && same(key, *cell)) {
      return cell->lvalue();
    }
    idx++;
  }
  cell->set(key, hash, 0);

  m_size++;
  m_useSize++;
  if (overloaded()) {
    growTable();
    return (*this)[key];
  }
  return cell->lvalue();
}

void HphpMap::erase(CVarRef key) {
  return erase(key, hash(key));
}

void HphpMap::erase(CVarRef key, int64 preHash /* = -1 */) {
  ASSERT(preHash < 0 || preHash == hash(key));

  int64 idx = preHash >= 0 ? preHash : hash(key);
  int64 hash = idx;
  while (true) {
    idx &= m_table.size() - 1;
    Cell &cell = m_table[idx];
    if (cell.empty()) {
      return;
    }    if (cell.hash() == hash && same(key, cell)) {
      cell.erase();
      m_size--;
      return;
    }
    idx++;
  }
}

void HphpMap::growTable() {
  size_t size = m_table.size();
  uint newsize = size * 2;
  HphpVector<Cell> newTable(newsize);
  for (uint i = 0; i < size; i++) {
    const Cell &cell = m_table[i];
    if (cell.alive()) {
      insertCell(cell, newTable);
    }
  }
  m_table.swap(newTable);
}

void HphpMap::insertCell(const Cell &oldCell, HphpVector<Cell> &table,
                         bool copy /* = false */) {
  int64 idx = oldCell.hash();
  uint mask = table.size() - 1;
  ASSERT((mask & (mask+1)) == 0);
  while (true) {
    idx &= mask;
    Cell &cell = table[idx];
    if (cell.empty()) {
      cell.set(oldCell, copy);
      return;
    }
    idx++;
  }
}

void HphpMap::dump() {
  printf("Map: tsize=%u, useSize=%u, size=%u\n", m_table.size(),
         (uint)m_useSize, (uint)m_size);
  for (uint i = 0; i < m_table.size(); i++) {
    Cell& cell = m_table[i];
    if (cell.empty()) {
      continue;
    }
    if (cell.value() == -1) {
      printf("Deleted at %u:\n", i);
    } else {
      printf("Entry at %u:\n", i);
      cell.key().dump();
      printf("to %d\n", cell.value());
    }
  }
}

#else
// Old HphpMap implementation
IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(HphpMapNode);

void HphpMapNode::dump() {
  m_key.dump();
  printf("value: %d\n", m_value);
}

HphpMapConstIterator::HphpMapConstIterator(const HphpMapIterator& it)
  : m_node(it.get()), m_map(it.getmap()) {
}

HphpMapConstIterator& HphpMapConstIterator::operator++() {
  if (m_node->m_next) {
    m_node = m_node->m_next;
  } else {
    size_t tsize =  m_map->m_table.size();
    Node *cur = NULL;
    for (uint idx = HphpMap::hash(m_node->key()) % tsize + 1;
         idx < tsize && !cur; idx++) {
      cur = m_map->m_table[idx];
    }
    m_node = cur;
  }
  return *this;
}

HphpMapIterator& HphpMapIterator::operator++() {
  if (m_node->m_next) {
    m_node = m_node->m_next;
  } else {
    size_t tsize =  m_map->m_table.size();
    Node* cur = NULL;
    for (uint idx = HphpMap::hash(m_node->key()) % tsize + 1;
         idx < tsize && !cur; idx++) {
      cur = m_map->m_table[idx];
    }
    m_node = cur;
  }
  return *this;
}

int64 HphpMap::hash(CVarRef s) {
  int64 hash;
  switch (s.getType()) {
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    hash = hash_int64(s.toInt64());
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

bool HphpMap::same(CVarRef s1, CVarRef s2) {
  DataType t1 = s1.getType();
  DataType t2 = s2.getType();
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
    return s1.toInt64() == s2.toInt64();
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
    return string_strcmp(s1d, s1l, s2d, s2l) == 0;
  }
}

void HphpMap::copy(const HphpMap& map) {
  ASSERT(&map != this);

  m_table.resize(map.m_table.size());
  for (const_iterator it = map.begin(); it != map.end(); ++it) {
    Node* n = NEW(Node)(it->key(), it->value(), NULL);
    insertNode(n, m_table);
  }
  m_size = map.m_size;
}

void HphpMap::clear() {
  Node *node, *next;
  size_t size = m_table.size();
  for (uint i = 0; i < size; i++) {
    for (node = m_table[i]; node; node = next) {
      next = node->m_next;
      DELETE(Node)(node);
    }
    m_table[i] = NULL;
  }
  m_size = 0;
}

bool HphpMap::insert(CVarRef key, int value, int &oldValue,
                     int64 preHash /* = -1 */) {
  ASSERT(preHash < 0 || preHash == hash(key));

  uint64 idx = (preHash >= 0 ? preHash : hash(key)) % m_table.size();
  Node *head = m_table[idx];
  for (Node* node = head; node; node = node->m_next) {
    if (same(node->key(), key)) {
      oldValue = node->value();
      return false;
    }
  }

  Node *newNode = NEW(Node)(key, value, head);
  m_table[idx] = newNode;
  oldValue = value;
  m_size++;
  if (overloaded()) {
    growTable();
  }
  return true;
}

int& HphpMap::operator[](CVarRef key) {
  uint64 idx = hash(key) % m_table.size();
  Node* head = m_table[idx];
  for (Node* node = head; node; node = node->m_next) {
    if (same(node->key(), key)) {
      return node->lvalue();
    }
  }

  Node* newNode = NEW(Node)(key, 0, head);
  m_table[idx] = newNode;

  m_size++;
  if (overloaded()) {
    growTable();
  }
  return newNode->lvalue();
}

bool HphpMap::find(CVarRef key, int &value, int64 preHash /* = -1 */) const {
  ASSERT(preHash < 0 || preHash == hash(key));

  uint64 idx = (preHash >= 0 ? preHash : hash(key)) % m_table.size();
  for (Node* node = m_table[idx]; node; node = node->m_next) {
    if (same(node->key(), key)) {
      value = node->value();
      return true;
    }
  }
  return false;
}

void HphpMap::erase(CVarRef key) {
  return erase(key, hash(key));
}

void HphpMap::erase(CVarRef key, int64 preHash /* = -1 */) {
  ASSERT(preHash < 0 || preHash == hash(key));

  uint64 idx = (preHash >= 0 ? preHash : hash(key)) % m_table.size();
  Node* prev = NULL;
  for (Node* node = m_table[idx];
       node;
       prev = node,
       node = node->m_next) {
    if (same(node->key(), key)) {
      if (prev) {
        prev->m_next = node->m_next;
      } else {
        m_table[idx] = node->m_next;
      }
      DELETE(Node)(node);
      m_size--;
      return;
    }
  }
}

void HphpMap::growTable() {
  size_t size = m_table.size();
  uint newsize = size * 2;
  HphpVector<Node*> newTable(newsize);
  for (uint i = 0; i < size; i++) {
    Node* next;
    for (Node* node = m_table[i];
         node;
         node = next) {
      next = node->m_next;
      insertNode(node, newTable);
    }
  }
  m_table.swap(newTable);
}

void HphpMap::insertNode(Node* node, HphpVector<Node*> &table) {
  uint idx = hash(node->key()) % table.size();
  Node* next = table[idx];
  node->m_next = next;
  table[idx] = node;
}

#endif
