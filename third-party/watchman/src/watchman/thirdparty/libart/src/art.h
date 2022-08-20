#ifndef ART_H
#define ART_H
#include "config.h" // @manual
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#define ART_MAX_PREFIX_LEN 10u

/**
 * Main struct, points to root.
 */
template <typename ValueType, typename KeyType = std::string>
struct art_tree {
  struct Leaf;
  struct Node;
  enum Node_type : uint8_t { NODE4 = 1, NODE16, NODE48, NODE256 };

  struct Deleter {
    void operator()(Node* node) const;
  };
  using NodePtr = std::unique_ptr<Node, Deleter>;
  using LeafPtr = std::unique_ptr<Leaf>;

  /**
   * This struct is included as part
   * of all the various node sizes
   */
  struct Node {
    Node_type type;
    uint8_t num_children{0};
    uint32_t partial_len{0};
    unsigned char partial[ART_MAX_PREFIX_LEN];

    virtual ~Node() = default;
    explicit Node(Node_type type);
    Node(Node_type type, const Node& other);
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    Leaf* maximum() const;
    Leaf* minimum() const;
    virtual NodePtr* findChild(unsigned char c) = 0;

    // Returns the number of prefix characters shared between the key and node.
    uint32_t checkPrefix(
        const unsigned char* key,
        uint32_t key_len,
        uint32_t depth) const;
    // Calculates the index at which the prefixes mismatch
    uint32_t prefixMismatch(
        const unsigned char* key,
        uint32_t key_len,
        uint32_t depth) const;

    virtual void addChild(NodePtr& ref, unsigned char c, NodePtr&& child) = 0;
    virtual NodePtr removeChild(NodePtr& ref, unsigned char c, NodePtr* l) = 0;
  };

  struct Node4;
  struct Node16;
  struct Node48;
  struct Node256;

  // Helper for dispatching to the correct node type
  union node_ptr {
    Node* n;
    Node4* n4;
    Node16* n16;
    Node48* n48;
    Node256* n256;
  };

  // const flavor of the above
  union cnode_ptr {
    const Node* n;
    const Node4* n4;
    const Node16* n16;
    const Node48* n48;
    const Node256* n256;
  };

  static inline bool IS_LEAF(const Node* x) {
    return uintptr_t(x) & 1;
  }

  static inline Leaf* LEAF_RAW(const Node* x) {
    return (Leaf*)((void*)((uintptr_t(x) & ~1)));
  }

  static inline NodePtr LeafToNode(LeafPtr&& leaf) {
    return NodePtr((Node*)(uintptr_t(leaf.release()) | 1));
  }

  static inline LeafPtr NodeToLeaf(NodePtr&& node) {
    return LeafPtr(LEAF_RAW(node.release()));
  }

  static inline unsigned char
  keyAt(const unsigned char* key, uint32_t key_len, uint32_t idx);
  static inline unsigned char
  keyAt(const char* key, uint32_t key_len, uint32_t idx);

  /**
   * Small node with only 4 children
   */
  struct Node4 : public Node {
    unsigned char keys[4];
    std::array<NodePtr, 4> children;

    Node4();
    explicit Node4(Node16&& n16);
    void addChild(NodePtr& ref, unsigned char c, NodePtr&& child) override;
    NodePtr removeChild(NodePtr& ref, unsigned char c, NodePtr* l) override;
    NodePtr* findChild(unsigned char c) override;
  };

  /**
   * Node with 16 children
   */
  struct Node16 : public Node {
    unsigned char keys[16];
    std::array<NodePtr, 16> children;

    Node16();
    explicit Node16(Node4&& n4);
    explicit Node16(Node48&& n48);
    void addChild(NodePtr& ref, unsigned char c, NodePtr&& child) override;
    NodePtr removeChild(NodePtr& ref, unsigned char c, NodePtr* l) override;
    NodePtr* findChild(unsigned char c) override;
  };

  /**
   * Node with 48 children, but
   * a full 256 byte field.
   */
  struct Node48 : public Node {
    unsigned char keys[256];
    std::array<NodePtr, 48> children;

    Node48();
    explicit Node48(Node16&& n16);
    explicit Node48(Node256&& n256);
    void addChild(NodePtr& ref, unsigned char c, NodePtr&& child) override;
    NodePtr removeChild(NodePtr& ref, unsigned char c, NodePtr* l) override;
    NodePtr* findChild(unsigned char c) override;
  };

  /**
   * Full node with 256 children
   */
  struct Node256 : public Node {
    std::array<NodePtr, 256> children;

    Node256();
    explicit Node256(Node48&& n48);
    void addChild(NodePtr& ref, unsigned char c, NodePtr&& child) override;
    NodePtr removeChild(NodePtr& ref, unsigned char c, NodePtr* l) override;
    NodePtr* findChild(unsigned char c) override;
  };

  /**
   * Represents a leaf. These are
   * of arbitrary size, as they include the key.
   */
  struct Leaf {
    KeyType key;
    ValueType value;

    template <typename... Args>
    Leaf(const KeyType& key, Args&&... args);

    bool matches(const unsigned char* key, uint32_t key_len) const;
    bool matches(const KeyType& key) const;

    uint32_t longestCommonPrefix(const Leaf* other, uint32_t depth) const;
    bool prefixMatches(const unsigned char* prefix, uint32_t prefix_len) const;
    inline unsigned char keyAt(uint32_t idx) const;
  };

  art_tree();
  ~art_tree();

  art_tree(const art_tree&) = delete;
  art_tree(art_tree&& other) noexcept;

  inline uint64_t size() const {
    return size_;
  }

  void clear();

  /**
   * Construct a new value in-place in the ART tree.
   * The arguments are forwarded to the ValueType constructor
   */
  template <typename... Args>
  void insert(const KeyType& key, Args&&... args);

  /**
   * Deletes a value from the ART tree
   * @arg key The key
   * @arg key_len The length of the key
   * @return true if the item was erased, false otherwise.
   */
  LeafPtr erase(const unsigned char* key, uint32_t key_len);
  LeafPtr erase(const KeyType& key);

  /**
   * Searches for a value in the ART tree
   * @arg key The key
   * @arg key_len The length of the key
   * @return NULL if the item was not found, otherwise
   * the value pointer is returned.
   */
  ValueType* search(const unsigned char* key, uint32_t key_len) const;
  ValueType* search(const KeyType& key) const;

  /**
   * Searches for the longest prefix match for the input key.
   * @arg key The key
   * @arg key_len The length of the key
   * @return NULL if no match was not found, otherwise
   * the leaf node with the longest matching prefix is returned.
   */
  Leaf* longestMatch(const unsigned char* key, uint32_t key_len) const;

  /**
   * Returns the minimum valued leaf
   * @return The minimum leaf or NULL
   */
  Leaf* minimum() const;

  /**
   * Returns the maximum valued leaf
   * @return The maximum leaf or NULL
   */
  Leaf* maximum() const;

  /**
   * Iterates through the entries pairs in the map,
   * invoking a callback for each. The call back gets a
   * key, value for each and returns an integer stop value.
   * If the callback returns non-zero, then the iteration stops.
   * @arg cb The callback function to invoke
   * @arg data Opaque handle passed to the callback
   * @return 0 on success, or the return of the callback.
   */
  template <typename Func>
  int iter(Func&& func);

  /**
   * Iterates through the entries pairs in the map,
   * invoking a callback for each that matches a given prefix.
   * The call back gets a key, value for each and returns an integer stop value.
   * If the callback returns non-zero, then the iteration stops.
   * @arg prefix The prefix of keys to read
   * @arg prefix_len The length of the prefix
   * @arg cb The callback function to invoke
   * @arg data Opaque handle passed to the callback
   * @return 0 on success, or the return of the callback.
   */
  template <typename Func>
  int iterPrefix(const unsigned char* prefix, uint32_t prefix_len, Func&& func);

 private:
  NodePtr root_;
  uint64_t size_;

  template <typename... Args>
  void recursiveInsert(
      NodePtr& ref,
      const KeyType& key,
      uint32_t depth,
      bool& replaced,
      Args&&... args);
  NodePtr recursiveDelete(
      NodePtr& ref,
      const unsigned char* key,
      uint32_t key_len,
      uint32_t depth);

  template <typename Func>
  int recursiveIter(Node* n, Func& func);
};

#include "art-inl.h"
#endif
