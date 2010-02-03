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

#ifndef __HPHP_UTIL_LFU_TABLE_H__
#define __HPHP_UTIL_LFU_TABLE_H__

#include <util/base.h>
#include <util/lock.h>
#include <util/atomic.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template<class K, class V>
class LFUNullDestructor {
public:
  void operator()(const K &k, V &v) {}
};

/**
 * A lookup table with a maximum size. Once it reaches maximum size,
 * inserts will evict the least frequently used item.
 *
 * It consists of 3 parts: a queue, a heap, and a map.
 * The map is used for lookup by key. The queue is a staging area for
 * values that are too new to have an accurate frequency, and the heap
 * serves as a priority queue for looking up the element that has
 * the lowest frequency.
 *
 * Frequencies are updated only when the hit count hits a multiple of the
 * period option. When a frequency is updated, the heap is also updated
 * to maintain the heap property. Frequency should be locked by the queue
 * lock since it must not change during heap operations.
 *
 * There are two locks that make it thread safe. There is read/write lock
 * for the map, and a lock for the queue/heap. The acquisition order
 * is map then queue/heap.
 *
 * An element is in the map if it is in the queue xor the heap.
 * If max capacity is 0, then all queue operations are disabled.
 * Elements can be added to the map but not the queue.
 */
template<class K, class V, class H, class E,
         class D = LFUNullDestructor<K, V> >
class LFUTable {
public:
  class LFUTableIterator {
  public:
    LFUTableIterator(typename LFUTable::Map::iterator &it) : m_it(it) {}
    LFUTableIterator(const LFUTableIterator &it) : m_it(it.m_it) {}
    const K &first() const {
      return m_it->first;
    }
    V &second() const {
      return m_it->second->val;
    }
    LFUTableIterator &operator++() {
      ++m_it;
      return *this;
    }
    bool operator==(const LFUTableIterator &it) const {
      return m_it == it.m_it;
    }
    bool operator!=(const LFUTableIterator &it) const {
      return m_it != it.m_it;
    }
  private:
    friend class LFUTable::LFUTableConstIterator;
    friend class LFUTable;
    typename LFUTable::Map::iterator m_it;
  };
  class LFUTableConstIterator {
  public:
    LFUTableConstIterator(typename LFUTable::Map::const_iterator &it)
      : m_it(it) {}
    LFUTableConstIterator(const LFUTableConstIterator &it) : m_it(it.m_it) {
    }
    LFUTableConstIterator(const LFUTableIterator &it) : m_it(it.m_it) {
    }
    const K &first() const {
      return m_it->first;
    }
    const V &second() const {
      return m_it->second->val;
    }
    LFUTableConstIterator &operator++() {
      ++m_it;
      return *this;
    }
    bool operator==(const LFUTableConstIterator &it) const {
      return m_it == it.m_it;
    }
    bool operator!=(const LFUTableConstIterator &it) const {
      return m_it != it.m_it;
    }
  private:
    friend class LFUTable;
    typename LFUTable::Map::const_iterator m_it;
  };
  /**
   * AtomicReader is used to perform a lookup under the internal
   * lock of the table. It takes the read lock.
   */
  class AtomicReader {
  public:
    virtual ~AtomicReader() {}
    virtual void read(const K &key, const V &val) = 0;
  };
  /**
   * AtomicUpdater is used to perform a update/insert under the internal
   * lock of the table. It takes the write lock.
   * The update method is given a reference to the value in the table and
   * a bool that specifies if the element was newly added.
   * It the method returns false, the element is deleted.
   */
  class AtomicUpdater {
  public:
    virtual ~AtomicUpdater() {}
    virtual bool update(const K &key, V &val, bool newlyCreated) = 0;
  };

public:
  LFUTable(time_t maturity, size_t maxCap, int updatePeriod)
    : m_head(NULL), m_tail(NULL), m_immortalCount(0),
      m_maturityThreshold(maturity), m_maximumCapacity(maxCap),
      m_updatePeriod(updatePeriod) {}
  ~LFUTable() {
    clear();
  }

  typedef LFUTableConstIterator const_iterator;
  typedef LFUTableIterator iterator;

  const_iterator begin() const {
    return const_iterator(m_map.begin());
  }
  const_iterator end() const {
    return const_iterator(m_map.end());
  }
  iterator begin() {
    typename Map::iterator it = m_map.begin();
    return iterator(it);
  }
  iterator end() {
    typename Map::iterator it = m_map.end();
    return iterator(it);
  }

  void insert(const K &k, const V &v) {
    WriteLock lock(m_mapLock);
    if (_contains(k)) {
      _erase(k);
    }
    _makeRoom();
    // Add to the map
    typename Map::iterator ins = m_map.insert(std::pair<const K, Node*>(k,NULL))
      .first;
    Node *n = new Node(ins->first, v);
    ins->second = n;
    // Add to queue
    insertQueue(n);
  }

  bool atomicRead(const K &k, AtomicReader &reader) {
    ReadLock lock(m_mapLock);
    Node *n = _getNode(k, false);
    if (n) {
      reader.read(n->key, n->val);
      return true;
    }
    return false;
  }

  void atomicForeach(AtomicReader &reader) {
    ReadLock lock(m_mapLock);
    for (typename Map::const_iterator it = m_map.begin();
         it != m_map.end(); ++it) {
      reader.read(it->first, it->second->val);
    }
  }

  void atomicUpdate(const K &k, AtomicUpdater &updater, bool createNew,
                    bool immortal = false) {
    WriteLock lock(m_mapLock);
    bool erase = false;
    Node *n = _getNode(k, false);
    bool created = false;
    if (!n && createNew) {
      n = _createNode(k, immortal);
      created = true;
    }
    if (n) {
      erase = updater.update(n->key, n->val, created);
    }
    if (erase) {
      _erase(k);
    }
  }

  void erase(const K &k) {
    WriteLock lock(m_mapLock);
    _erase(k);
  }
  void erase(const iterator &it) {
    WriteLock lock(m_mapLock);
    _erase(it);
  }

  bool lookup(const K &k, V &result) {
    ReadLock lock(m_mapLock);
    Node *n = _getNode(k, false);
    if (n) {
      result = n->val;
      return true;
    }
    return false;
  }
  iterator find(const K &k) {
    ReadLock lock(m_mapLock);
    typename Map::iterator it = m_map.find(k);
    if (it != m_map.end()) {
      Node *n = it->second;
      _bumpNode(n);
    }
    return iterator(it);
  }
  V &operator[](const K &k) {
    WriteLock lock(m_mapLock);
    return _getNode(k, true)->val;
  }

  size_t size() const {
    return m_map.size();
  }
  size_t maximumCapacity() const {
    return m_maximumCapacity;
  }
  size_t immortalCount() const {
    return m_immortalCount;
  }

  void clear() {
    WriteLock lock(m_mapLock);
    typename Map::iterator it = m_map.begin();
    while (it != m_map.end()) {
      typename Map::iterator cit = it++;
      Node *n = cit->second;
      if (n->immortal) {
        // Trade immortal for mortal
        ++m_maximumCapacity;
        --m_immortalCount;
      } else {
        removeQueue(cit->second);
      }
      m_map.erase(cit);
      delete n;
    }
    ASSERT(m_head == NULL);
    ASSERT(m_tail == NULL);
    ASSERT(m_heap.size() == 0);
  }

  bool check() {
    WriteLock lock(m_mapLock);
    Lock qlock(m_queueLock);

    bool fail = false;
    Node *prev = NULL;
    Node *n = m_head;
    while (n) {
      if (m_map.find(n->key) == m_map.end()) {
        fail = true;
        Logger::Error("Value in queue not in map");
        ASSERT(!fail);
      }
      if (n->prev != prev) {
        fail = true;
        Logger::Error("Queue list corrupted");
        ASSERT(!fail);
      }
      prev = n;
      n = n->next;
      if (!n && prev != m_tail) {
        fail = true;
        Logger::Error("Queue tail incorrect");
        ASSERT(!fail);
      }
    }
    uint hsize = m_heap.size();
    for (uint i = 0; i < hsize; i++) {
      Node *n = m_heap[i];
      if (m_map.find(n->key) == m_map.end()) {
        fail = true;
        Logger::Error("Value in queue not in heap");
        ASSERT(!fail);
      }
      size_t child = (i+1) * 2;
      if (child <= hsize && n->frequency() > m_heap[child-1]->frequency()) {
        fail = true;
        Logger::Error("Heap property violated");
        ASSERT(!fail);
      }
      child++;
      if (child <= hsize && n->frequency() > m_heap[child-1]->frequency()) {
        fail = true;
        Logger::Error("Heap property violated");
        ASSERT(!fail);
      }
    }
    return !fail;
  }

private:
  class Node {
  public:
    Node(const K &k) : key(k), prev(NULL), next(NULL), hits(0), heapIndex(0),
                       immortal(false), m_freq(0), m_timestamp(time(NULL)) {}
    Node(const K &k, const V &v)
      : key(k), val(v), prev(NULL), next(NULL),  hits(0), heapIndex(0),
        immortal(false), m_freq(0), m_timestamp(time(NULL))  {}
    ~Node() {
      D d;
      d(key, val);
    }
    const K &key;
    V val;
    Node *prev;
    Node *next;
    uint64 hits;
    uint heapIndex;
    bool immortal;


    // Frequency updates must be accompanied by heap updates since otherwise
    // it may break the heap property. It should probably be under lock too.
    bool updateFrequency() {
      double lifetime = time(NULL) - m_timestamp;
      double oldFreq = m_freq;
      if (lifetime > 0) {
        m_freq = hits / lifetime;
      }
      return m_freq > oldFreq;
    }
    double frequency() const {
      return m_freq;
    }
    time_t timestamp() {
      return m_timestamp;
    }
  private:
    double m_freq;
    time_t m_timestamp;
  };

private:
  //////////////////////////////////////////////////////////////////////////////
  // These methods are to be used when the map lock is already acquired.
  bool _contains(const K &k) {
    return m_map.find(k) != m_map.end();
  }
  void _erase(const K &k) {
    typename Map::iterator it = m_map.find(k);
    if (it != m_map.end()) {
      _erase(it);
    }
  }
  void _erase(const iterator &it) {
    Node *n = it.m_it->second;
    m_map.erase(it.m_it);
    if (n->immortal) {
      // We trade an immortal for a mortal value
      --m_immortalCount;
      ++m_maximumCapacity;
    } else {
      removeQueue(n);
    }
    delete n;
  }

  void _bumpNode(Node *n) {
    if (!m_maximumCapacity) return;
    if ((atomic_add(n->hits, (uint64)1) + 1) % m_updatePeriod == 0) {
      Lock lock(m_queueLock);
      // hits could have increased between incrementing and taking the lock,
      // but it does not matter.
      // It is also possible (if unlikely) that two threads could be trying to
      // do this update, but that doesn't matter either.
      bool increase = n->updateFrequency();
      _updateQueue(n, increase);
    }
  }
  void _makeRoom() {
    if (!m_maximumCapacity) return;
    while (size() - m_immortalCount >= m_maximumCapacity) {
      Node *m = popQueue();
      ASSERT(m);
      m_map.erase(m->key);
      delete m;
    }
  }
  Node *_getNode(const K &k, bool force, bool immortal = false) {
    typename Map::iterator it = m_map.find(k);
    if (it != m_map.end()) {
      _bumpNode(it->second);
      return it->second;
    }
    if (force) {
      return _createNode(k, immortal);
    } else {
      return NULL;
    }
  }

  Node *_createNode(const K &k, bool immortal = false) {
    if (immortal) {
      ++m_immortalCount;
    } else {
      _makeRoom();
    }
    // Add to the map
    typename Map::iterator ins = m_map.insert(std::pair<const K, Node*>(k,NULL))
      .first;
    Node *n = new Node(ins->first);
    ins->second = n;
    n->immortal = immortal;
    if (!immortal) {
      // Add to queue
      insertQueue(n);
    }
    return n;
  }

  void _dumpStatus() {
    std::cout << "in map:\n";
    for (typename Map::const_iterator it = m_map.begin(); it != m_map.end();
         ++it) {
      std::cout << it->first << " : " << it->second->frequency() << "\n";
    }
    if (!m_maximumCapacity) return;
    std::cout << "in queue:\n";
    Node *n = m_head;
    while (n) {
      std::cout << n->key << " : " << n->frequency() << "\n";
      n = n->next;
    }
    std::cout << "in heap\n";
    for (uint i = 0; i < m_heap.size(); i++) {
      std::cout << m_heap[i]->key << " : " << m_heap[i]->frequency() << "\n";
    }
  }

private:
  //////////////////////////////////////////////////////////////////////////////
  // Queue interface methods

  // Add item to queue
  void insertQueue(Node *item) {
    if (!m_maximumCapacity) return;
    Lock lock(m_queueLock);
    // Add to head of list
    item->heapIndex = 0;
    item->prev = NULL;
    item->next = m_head;
    if (m_head) m_head->prev = item;
    m_head = item;
    if (!m_tail) m_tail = item;
    _shiftMature();
  }

  void removeQueue(Node *item) {
    if (!m_maximumCapacity) return;
    Lock lock(m_queueLock);
    if (item == m_head) {
      m_head = item->next;
    }
    if (item == m_tail) {
      m_tail = item->prev;
    }
    if (item->prev) {
      item->prev->next = item->next;
    }
    if (item->next) {
      item->next->prev = item->prev;
    }
    item->prev = item->next = NULL;
    if (item->heapIndex != 0) {
      int pos = item->heapIndex;
      int last = m_heap.size();
      item->heapIndex = 0;
      m_heap[pos-1] = m_heap[last-1];
      m_heap[pos-1]->heapIndex = pos;
      m_heap.pop_back();
      _heapifyDown(pos);
    }
  }
  // Pop the minimum
  Node *popQueue() {
    if (!m_maximumCapacity) return NULL;
    Lock lock(m_queueLock);
    _shiftMature();
    if (m_heap.size() == 0) {
      Node *res = m_tail;
      if (!res) return NULL;
      if (res->prev) {
        res->prev->next = NULL;
      }
      m_tail = res->prev;
      res->prev = NULL;
      if (res == m_head) m_head = NULL;
      return res;
    }
    Node *res = m_heap[0];
    res->heapIndex = 0;
    m_heap[0] = m_heap[m_heap.size() - 1];
    m_heap[0]->heapIndex = 1;
    m_heap.pop_back();
    _heapifyDown(1);
    return res;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Queue helper methods

  void _shiftMature() {
    // Move mature items to heap
    time_t now = time(NULL);
    while (m_tail && ((now - m_tail->timestamp()) >= m_maturityThreshold)) {
      Node *last = m_tail;
      if (last->prev) {
        last->prev->next = NULL;
      }
      m_tail = last->prev;
      if (m_head == last) m_head = NULL;
      last->prev = NULL;
      last->updateFrequency();
      _heapInsert(last);
    }
  }

  void _updateQueue(const Node *item, bool increase) {
    if (!m_maximumCapacity) return;
    if (item->heapIndex != 0) {
      if (increase) {
        _heapifyDown(item->heapIndex);
      } else {
        _heapifyUp(item->heapIndex);
      }
    }
  }

  void _heapInsert(Node *item) {
    m_heap.push_back(item);
    item->heapIndex = m_heap.size();
    int pos = m_heap.size();
    _heapifyUp(pos);
  }

  void _heapifyUp(int pos) {
    while (pos != 1) {
      Node *&child = m_heap[pos - 1];
      Node *&parent = m_heap[pos/2 - 1];
      if (parent->frequency() > child->frequency()) {
        swap(child, parent);
        child->heapIndex = pos;
        parent->heapIndex = pos/2;
        pos = pos/2;
      } else {
        break;
      }
    }
  }

  void _heapifyDown(int pos) {
    int size = m_heap.size();
    for (;;) {
      int child;
      if (pos * 2 > size) break;
      if (pos * 2 + 1 > size || (m_heap[pos*2 - 1]->frequency() <
                                 m_heap[pos*2]->frequency())) {
        child = pos * 2;
      } else {
        child = pos * 2 + 1;
      }
      if (m_heap[pos - 1]->frequency() > m_heap[child - 1]->frequency()) {
        swap(m_heap[pos - 1], m_heap[child - 1]);
        m_heap[pos - 1]->heapIndex = pos;
        m_heap[child - 1]->heapIndex = child;
        pos = child;
      } else {
        break;
      }
    }
  }

private:
  // The queue. A doubly linked list since I need to remove elements at will.
  Node *m_head;
  Node *m_tail;
  // The heap
  std::vector<Node*> m_heap;
  // The map
  typedef hphp_hash_map<K, Node*, H, E> Map;
  //typedef std::map<K, Node*> Map;
  Map m_map;
  size_t m_immortalCount;

  // Locks in acquisition order
  ReadWriteMutex m_mapLock;
  Mutex m_queueLock;

  // Options
  time_t m_maturityThreshold;
  size_t m_maximumCapacity;
  int m_updatePeriod;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_UTIL_LFU_TABLE_H__
