/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_UTIL_RADIX_MAP_H_
#define incl_HPHP_UTIL_RADIX_MAP_H_

#include "hphp/util/compilation-flags.h"
#include "hphp/util/assertions.h"

namespace HPHP {

/*
 * RadixMap stores blocks of memory represented by [ptr,size), where both
 * ptr and size have at least LgAlign zeroed low-bits. It supports
 * insert, erase, get, iterate, and find. find(addr) retreives
 * the [ptr,size) block that contains addr.
 *
 * Internally, the tree consists of nodes with Radix = 2^LgRadix slots each;
 * larger values of LgRadix result in a wider, shallower tree. Each tree level
 * represents LgRadix number of bits of the pointer stored at that level,
 * with each slot representing a unique group of bits of the pointer.
 * Together with LgAlign, the bits of a pointer are treated as follows:
 *
 *   [prefix][LgRadix]...[LgRadix][LgAlign]
 *   ^-- msb                        lsb --^
 *
 * The low LgAlign bits are ignored, and each interior LgRadix bits are
 * used as the index into the node at that level. Low order bits index
 * into leaf nodes in the tree. The highest order index accesses the root
 * node of the tree. All pointers in the tree share the same common prefix,
 * which is not used other than to recreate pointers and detect when the tree
 * needs to be made deeper.
 */
template<class T, size_t LgAlign = 3, size_t LgRadix = 4>
struct RadixMap {
  static_assert(LgAlign >= 1, "need one ptr/size bit for internal use");
  static_assert(std::is_pointer<T>::value, "T must be a pointer type");
  static constexpr size_t Radix = 1<<LgRadix;
  static constexpr size_t Align = 1<<LgAlign;

  // initial root_prefix has bits set that cannot occur in any ptr
  static constexpr size_t EmptyPrefix = ~0ul;

  // Implementation notes.
  //
  // All accessors work by descending the tree, doing their work.
  // by convention, when "scale" is tracked, it is the tree level (starting
  // at 0 for nodes that can't have children) times LgRadix. Each slot
  // represents (1<<scale)+LgAlign bytes.
  //
  // Each slot can be in one of three states:
  //   empty  - 0
  //   leaf   - size, representing a [ptr,size) rmap entry
  //   child  - pointer to child node
  //
  // root_prefix contains the high order bits common to every ptr.

  struct Node {
    Node() {}
    bool is_leaf(size_t i) const {
      return slots[i].size & 1;
    }
    bool is_child(size_t i) const {
      return !is_leaf(i) && slots[i].child != nullptr;
    }
    // Initialize slot i as a leaf. Caller must first call destroy_node()
    // if the slot previously contained a child pointer.
    void set_size(size_t i, size_t size) {
      assert(size != 0 && size % Align == 0);
      slots[i].size = size + 1; // +1 to set the leaf flag
    }
    void set_child(size_t i, Node* child) {
      assert(!is_child(i));
      slots[i].child = child;
    }
    void erase(size_t i) {
      assert(is_leaf(i));
      slots[i].size = 0;
    }
    // Return the child at slot i, or null if the slot doesn't hold a child
    // pointer. It's a bug to call this function on a leaf (ie a nonempty slot).
    Node* child(size_t i) const {
      assert(!is_leaf(i));
      return slots[i].child;
    }
    size_t size(size_t i) const {
      assert(is_leaf(i));
      return slots[i].size - 1; // -1 to remove the leaf flag.
    }
   public:
    union {
      Node* child;
      size_t size; // if leaf
    } slots[Radix];
  };

  // recursively destroy node's children (if any), then free node.
  NEVER_INLINE void destroy_node(Node* node, int scale) {
    assert(scale % LgRadix == 0);
    if (scale > LgRadix) {
      for (size_t i = 0; i < Radix; ++i) {
        if (node->is_child(i)) {
          destroy_node(node->child(i), scale - LgRadix);
        }
      }
    } else if (scale == LgRadix) {
      for (size_t i = 0; i < Radix; ++i) {
        if (node->is_child(i)) {
          free(node->child(i));
          --m_node_count;
        }
      }
    }
    assert(node != m_root || m_node_count == 1);
    free(node);
    --m_node_count;
  }

  ~RadixMap() {
    if (m_root) destroy_node(m_root, m_root_scale);
  }

  void clear() {
    if (m_root) {
      destroy_node(m_root, m_root_scale);
      m_root = nullptr;
      m_root_prefix = EmptyPrefix;
      m_root_scale = 0;
      m_node_count = 0;
    }
  }

  bool empty() const {
    return !m_root;
  }

  // Iteratively count the number of blocks currently stored.
  size_t countBlocks() const {
    size_t n = 0;
    iterate([&](T, size_t) { ++n; });
    return n;
  }

  /*
   * Insert [ptr,size). Assumes ptr not already present, and [ptr,size)
   * does not overlap any existing range.
   */
  void insert(T ptr, const size_t size) {
    assert(size >= Align);
    // valid pointers cannot have EmptyPrefix as their upper bits.
    assert(upper(toKey(ptr), m_root_scale) != EmptyPrefix);
    assert(uintptr_t(ptr) % Align == 0 && size % Align == 0);
    // this assert is too expensive to leave enabled by default.
    //assert(!m_root || !find(ptr).ptr);
    auto const k = toKey(ptr);
    // compute the max scale at which [ptr,size) would fit perfectly;
    // ie scale <= the lsb of both k and size/Align.
    auto max_scale = __builtin_ffsl(k | (size >> LgAlign)) - 1;
    auto n = m_root;
    // if necessary, make the tree deeper to expand addressable range
    while (upper(k, m_root_scale) != m_root_prefix ||
           m_root_scale + LgRadix <= max_scale) {
      if (!n) {
        n = m_root = make_node();
        // make tree deeper until block can be stored at its natural level
        while (m_root_scale + LgRadix <= max_scale) {
          m_root_scale += LgRadix;
        }
        m_root_prefix = upper(k, m_root_scale);
        break;
      }
      // deepen tree while new pointer is out of root's range, or if
      // it should be stored at a higher level anyway.
      n = make_node();
      auto scale = m_root_scale + LgRadix;
      n->set_child(index(m_root_prefix, scale), m_root);
      m_root_prefix = upper(m_root_prefix, scale);
      m_root_scale = scale;
      m_root = n;
    }
    // walk down tree, insert new range as high as possible
    for (auto scale = m_root_scale;; scale -= LgRadix) {
      auto i = index(k, scale);
      auto child = n->child(i);
      if (scale <= max_scale) {
        if (child) {
          destroy_node(child, scale - LgRadix);
        }
        return n->set_size(i, size);
      }
      if (!child) {
        child = make_node();
        n->set_child(i, child);
      }
      n = child;
    }
    not_reached();
  }

  /*
   * Return the size associated with ptr, 0 if no range is found.
   * Ptr must be exactly equal the start of a range stored in the map.
   */
  size_t get(const void* ptr) const {
    // if !m_root, then m_root_prefix cannot match any pointer
    assert(m_root || (m_root_prefix == EmptyPrefix && m_root_scale == 0));
    auto const k = toKey(ptr);
    auto scale = m_root_scale;
    if (upper(k, scale) == m_root_prefix) {
      for (auto n = m_root; n != nullptr;) {
        auto i = index(k, scale);
        if (n->is_leaf(i)) {
          return lower(ptr, scale) == 0 ? n->size(i) : 0;
        }
        scale -= LgRadix;
        n = n->child(i);
      }
    }
    return 0;
  }

  /*
   * If ptr exactly equals the start of a range stored in the map, erase it
   * from the map and return its size. Otherwise do nothing and return 0.
   */
  size_t erase(const void* ptr) {
    // if !m_root, then m_root_prefix cannot match any pointer
    assert(m_root || (m_root_prefix == EmptyPrefix && m_root_scale == 0));
    auto const k = toKey(ptr);
    auto scale = m_root_scale;
    // if the tree is empty, this check will fail
    if (upper(k, scale) == m_root_prefix) {
      for (auto n = m_root; n != nullptr;) {
        auto i = index(k, scale);
        if (n->is_leaf(i)) {
          if (lower(ptr, scale) == 0) {
            auto size = n->size(i);
            n->erase(i);
            return size;
          }
          return 0;
        }
        scale -= LgRadix;
        n = n->child(i);
      }
    }
    return 0;
  }

  /*
   * Iterate through the map in address order.
   */
  template<class Fn> void iterate(Fn fn) const {
    if (!m_root) return;
    for (size_t k = m_root_prefix, e = k + slot_size(m_root_scale + LgRadix);
         k < e;) {
      auto n = m_root;
      auto scale = m_root_scale;
      for (size_t i = index(k, scale); i < Radix;) {
        if (n->is_leaf(i)) {
          auto size = n->size(i);
          fn((T)(k << LgAlign), size);
          k += size >> LgAlign;
          i += size >> (scale + LgAlign);
        } else if (auto child = n->child(i)) {
          // down to next level without changing k
          n = child;
          i = index(k, scale -= LgRadix);
        } else {
          // move to start of next slot at this level
          k = (k + slot_size(scale)) & ~(slot_size(scale) - 1);
          ++i;
        }
      }
    }
  }

  /*
   * Return the entry enclosing ptr, or nullptr if no such entry exists.
   */
  struct FindResult { T ptr; size_t size; };
  FindResult find(const void* ptr) const {
    // if !m_root, then m_root_prefix cannot match any pointer
    assert(m_root || (m_root_prefix == EmptyPrefix && m_root_scale == 0));
    auto end = m_root_prefix + slot_size(m_root_scale + LgRadix) - 1;
    auto needle = toKey(ptr);
    for (auto k = std::min(needle, end);
         upper(k, m_root_scale) == m_root_prefix;) {
      auto n = m_root;
      auto scale = m_root_scale;
      for (ssize_t i = index(k, scale); i >= 0;) {
        if (n->is_leaf(i)) {
          k &= ~(slot_size(scale) - 1);
          auto size = n->size(i);
          return needle >= k + (size >> LgAlign) ? FindResult{nullptr, 0} :
                 FindResult{(T)(k << LgAlign), size};
        }
        if (auto child = n->child(i)) {
          // down to next level without changing k
          n = child;
          i = index(k, scale -= LgRadix);
        } else {
          // move to end of previous slot at this level
          k = (k & ~(slot_size(scale) - 1)) - 1;
          --i;
          assert(i==-1 || i == index(k,scale));
        }
      }
    }
    return {nullptr, 0};
  }

private:
  static size_t toKey(const void* ptr) {
    return size_t(ptr) >> LgAlign;
  }

  // extract middle bits; 0 = lowest LgRadix bits
  static int index(size_t k, size_t scale) {
    return (k >> scale) & (Radix - 1);
  }

  // extract upper scale+LgRadix bits
  static size_t upper(size_t k, size_t scale) {
    return k & ~(slot_size(scale + LgRadix) - 1);
  }

  // extract the lower scale+LgAlign bits
  static size_t lower(const void* ptr, size_t scale) {
    return uintptr_t(ptr) & ((1UL << (scale + LgAlign)) - 1);
  }

  // return the number of leaf slots represented by each slot at this level.
  // each leaf slot represents Align bytes.
  static size_t slot_size(size_t scale) {
    return 1UL << scale;
  }

  Node* make_node() {
    ++m_node_count;
    return new (calloc(1, sizeof(Node))) Node();
  }

private:
  Node* m_root{nullptr};
  size_t m_root_prefix{EmptyPrefix};
  int m_root_scale{0};
  int m_node_count{0};
};

}

#endif
