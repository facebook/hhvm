/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_HPHP_PHP7_CFG_H
#define incl_HPHP_PHP7_CFG_H

#include "hphp/php7/bytecode.h"

#include <folly/Unit.h>

#include <memory>
#include <vector>

namespace HPHP { namespace php7 {

struct Region;

/* A basic block of bytecode
 *
 * Each block has a sequence of non-exit instructions followed by a sequence of
 * exit instructions. Each block is also assigned an ID but this is mostly
 * ignored--blocks are usually identified by their pointer until they are
 * output to HHAS. Nevertheless, when output, no two blocks should have the
 * same numeric ID in one function
 */
struct Block {
#define EXIT(name) void emit(bc::name) = delete;
#define EXIT_LAST EXIT
  EXIT_OPS
#undef EXIT
#undef EXIT_LAST
  void emit(ExitOp op) = delete;
  /* Add an normal instruction to the end of this block.
   *
   * If the block already has at least one exit instruction, no more regular
   * instructions can be added */
  void emit(Bytecode bc) {
    assert(!exited);
    code.push_back(std::move(bc));
  }

  /* Add an exit instruction to the end of this block */
  void exit(ExitOp op) {
    exited = true;
    exits.push_back(std::move(op));
  }

  /* identifies this block in its function */
  uint64_t id;

  // code associated with this block
  std::vector<Bytecode> code;
  std::vector<ExitOp> exits;
  Region* region;
  bool exited{false};
};

/* A region is a way of keeping bytecode contiguous in the output unit and to
 * mark protected regions. Regions come in three flavors: fault-protected
 * regions (those with an associated fault funclet), exception-protected (with
 * an associated catch label), and vanilla (nothing special).
 *
 * All regions correspond to:
 * - a function body (vanilla)
 * - the body of a try (protected)
 * - the body of a catch (vanilla)
 * - a fault funclet (vanilla)
 *
 * A catch's region must have the same immediate parent as the try's region
 *
 * In addition, regions must nest completely: i.e. if two regions share any
 * bytecode, one must contain the other completely.
 *
 * It's up to the compiler to ensure that all blocks in a region are dominated
 * by a single block (i.e. they can be made into one contiguous chunk of code)
 */
struct Region {
  enum Kind {
    Entry,
    Protected,
    Catch,
  };

  explicit Region(Kind kind, Region* parent = nullptr)
    : kind(kind)
    , parent(parent) {}

  Kind kind;
  Region* parent;
  Block* handler{nullptr};
  std::vector<std::unique_ptr<Region>> children;

  void addChild(std::unique_ptr<Region> child) {
    child->parent = this;
    children.push_back(std::move(child));
  }

  bool containsBlock(const Block* blk) const {
    auto region = blk->region;
    while (region) {
      if (region == this) {
        return true;
      }
      region = region->parent;
    }

    return false;
  }
};

struct CFGVisitor {
  virtual ~CFGVisitor() = default;

  virtual void beginTry() = 0;
  virtual void beginCatch() = 0;
  virtual void endRegion() = 0;

  virtual void block(Block* blk) = 0;
};

/* A CFG represents a collection of blocks with a single entry and one or more
 * exits.
 *
 * Each CFG has a fallthrough exit that we can add code to by compising it with
 * other CFGs or instructions using `then` or one of its variants. We can use
 * this to sequence bytecodes or even multiple CFGs in a linear order:
 *
 *     CFG graph;
 *     graph.then(Int{2});
 *     graph.then(Int{5});
 *     graph.then(Add{});
 *     graph.then(Print{});
 *     graph.then(PopC{});
 *     // graph.entry now has code that prints 7
 *
 *     CFG return;
 *     return.then(Int{42});
 *     return.thenReturn(Flavor::Cell);
 *     // return now has code that returns 42;
 *
 *     graph.then(std::move(return)); // code that prints 7, then returns 42
 *
 * Each CFG tracks a continuation block which represents the point where new
 * code should be added afterwards. For code that has no definite place where
 * control goes afterwards (e.g. return statements, throws, etc.) there is no
 * continuation: trying to append code to these CFGs is a no-op.
 *
 * The API for this class is designed to be used as a fluent API: each call
 * designed to modify a CFG does the modification and returns an
 * rvalue-reference to `this`: this lets us either chain the call with another,
 * move the result, or just modify the graph in place. E.g. we can write:
 *
 *     return CFG()
 *       .then(Int{2})
 *       .then(Int{4});
 *
 * Branches can be inserted in the way you expect:
 *
 *     CFG condition = ...;
 *     return condition
 *       .branchNZ( ... another CFG here ...)
 *       .then( ... else case here ...);
 *
 * There's four kinds of exits from a CFG: the fallthrough (continuation),
 * returns, throws, breaks/continues, and jumps to a named label. Named labels
 * can be used both by the compiler to assist in building CFGs and to compile
 * gotos. Each CFG tracks the labels it defines and the linkages to missing
 * labels (or to returns, etc.) Internal labels should be stripped out before
 * other CFGs are joined with the CFG where they are defined. More on this can
 * be found along with the methods used to define and strip labels.
 *
 * Normal exits (return, break, continue, etc.) are also treated as links since
 * we sometimes need to intercept these in order to compile finally or breaks.
 *
 * A CFG can be wrapped up and made ready to output as assembly just by making
 * all the links to exits into actual exit instructions. Note that this should
 * only be done before inserting the CFG into a unit since it means composing
 * the CFG further will probably have unexpected results.
 */
struct CFG {
  struct ReturnTarget {
    bool operator==(const ReturnTarget& other) const {
      return other.flavor == flavor;
    }

    Flavor flavor;
  };
  enum LoopTarget {
    Break,
    Continue
  };
  struct LabelTarget {
    bool operator==(const LabelTarget& other) const {
      return other.name == name;
    }

    std::string name;
  };

  using LinkTarget = boost::variant<
    ReturnTarget,
    LoopTarget,
    LabelTarget
  >;

  /* Construct an empty CFG */
  CFG()
    : m_entry(makeBlock())
    , m_continuation(m_entry) {}

  /* Construct a CFG that contains only the given instruction */
  explicit CFG(Bytecode bc);

  /* Construct a CFG that contains the given bytecodes */
  /* implicit */ CFG(std::initializer_list<Bytecode> list);

  /* Construct a CFG that contains only an unresolved link to the given exit */
  /* implicit */ CFG(LinkTarget target);

  CFG(CFG&&) = default;
  CFG(const CFG&) = delete;

  CFG& operator=(CFG&&) = default;
  CFG& operator=(const CFG&) = delete;

  ~CFG() = default;

  /* Add a block to this CFG */
  Block* makeBlock();

  Block* entry() const { return m_entry; }

  /* Construct a CFG in one go using internal labels. This function is designed
   * to write aseembler-style code fluently.
   *
   * As an example:
   *    CFG::Labeled(
   *      "entry", {
   *        Int{0},
   *        SetL{"x"},
   *        PopC{}
   *      },
   *      "check", CFG({
   *        CGetL{"x"},
   *        Int{10},
   *        Lt{},
   *      }).branchNZ("body")
   *        .then("end"),
   *      "body", CFG({
   *        IncDecL{"x", PostInc},
   *        Print{},
   *        PopC{}
   *      }).then("check"),
   *      "end", CFG())
   *
   * Each block falls through to the next and all the labels are stripped at
   * the end.
   */
  template <typename ...Blocks>
  static CFG Labeled(Blocks... more) {
    std::unordered_map<std::string, Block*> labels;
    auto cfg = CFG::makeLabeled(labels, std::move(more)...);
    for(const auto& label : labels) {
      cfg.link(label.first, label.second);
      cfg.strip(label.first);
    }
    return cfg;
  }

  /* sequences a (CFG/instruction) into this CFG */
  CFG&& then(CFG cfg);
  CFG&& then(Bytecode bc);

  /* Inserts a jump to the given block address. The given block address should
   * either already be part of this CFG or about to be merged in */
  CFG&& thenJmp(Block* block);
  /* Add a branch to the given CFG into this CFG */
  CFG&& branchZ(CFG cfg);
  CFG&& branchNZ(CFG cfg);

  /* These variants insert a jump (possibly conditional) to the given label.
   * These are available as a convenience when writing code using CFG::Labeled
   */
  CFG&& then(const std::string& label);
  CFG&& branchZ(const std::string& label);
  CFG&& branchNZ(const std::string& label);
  CFG&& continueFrom(Block* block);
  CFG&& switchUnbounded(std::vector<CFG> exits);

  /* sequences a link into this CFG */
  CFG&& then(LinkTarget target);
  CFG&& thenThrow();
  CFG&& thenReturn(Flavor flavor);
  CFG&& thenContinue();
  CFG&& thenBreak();
  CFG&& thenLabel(const std::string& name);

  /* replace the given label and strip it out */
  CFG&& replace(const std::string& label, CFG cfg);

  /* Finalize all exits and raise errors if:
   *  - any continue or break hasn't been matched with a loop
   *  - any label is still unresolved
   *
   * This must be called on a CFG before inserting it into a unit, and no
   * sooner
   */
  CFG&& makeExitsReal();

  /* rewrite breaks and continues to resolve to these CFGs */
  CFG&& linkLoop(CFG breakTarget, CFG continueTarget);
  /* add a catch region with the given handler */
  CFG&& addExnHandler(CFG catchHandler);
  CFG&& addFinallyGuard(CFG guard);

  CFG&& inRegion(std::unique_ptr<Region> region);

  void visit(CFGVisitor&& visitor) const;

 private:
  /* replaces named labels with the given block */
  CFG&& link(const std::string& name, Block* dest);
  /* strip out the given label */
  CFG&& strip(const std::string& name);
  /* Combines two CFGs into one, re-labelling blocks as needed and resolving
   * links */
  void merge(CFG cfg);

  /* probably not what you want: actually emits an exit op instead of creating
   * a linkage to the eventual exit */
  CFG&& thenExitRaw(ExitOp op);

  /* A pending link
   *
   * Where a link is needed, a trampoline block is added to the CFG and a jump
   * to this trampoline is added.
   *
   * When the link is resolved, code is added to the trampoline to jump to the
   * real destination.
   *
   * This means the trampoline should always be a block that is *not* yet
   * exited.
   */
  struct Linkage {
    Block* trampoline;
    LinkTarget target;
  };

  template <class Function>
  void resolveLinks(Function&& f) {
    std::vector<Linkage> stillUnresolved;
    for (auto& link : m_unresolvedLinks) {
      if (Block* blk = f(link)) {
        link.trampoline->exit(bc::Jmp{blk});
      } else {
        stillUnresolved.push_back(std::move(link));
      }
    }
    m_unresolvedLinks = std::move(stillUnresolved);
  }

  /* A template to implement CFG::Labeled. Consumes one label and CFG and
   * sequences it into the resulting CFG; then adds the label to the set
   */
  template <typename ...Rest>
  static CFG makeLabeled(
      std::unordered_map<std::string, Block*>& labels,
      const std::string& name,
      CFG block, Rest... more) {
    auto cfg = block.then(CFG::makeLabeled(labels, std::move(more)...));
    labels.insert({name, cfg.m_entry});
    return cfg;
  }

  static CFG makeLabeled(std::unordered_map<std::string, Block*>& labels) {
    return {};
  }

  CFG&& self() {
    return std::move(*this);
  }

 public:
  uint64_t m_maxId{0};
  std::vector<std::unique_ptr<Block>> m_blocks;
  std::vector<std::unique_ptr<Region>> m_topRegions;
  std::unordered_map<std::string, Block*> m_labels;
  std::vector<Linkage> m_unresolvedLinks;
  Block* m_entry;
  Block* m_continuation;
};

}} // namespace HPHP::php7

#endif // incl_HPHP_PHP7_CFG_H
