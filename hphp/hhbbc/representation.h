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
#pragma once

#include <list>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/variant.hpp>

#include "hphp/util/atomic-vector.h"
#include "hphp/util/compact-vector.h"
#include "hphp/util/copy-ptr.h"
#include "hphp/util/sha1.h"

#include "hphp/runtime/base/user-attributes.h"
#include "hphp/runtime/vm/coeffects.h"
#include "hphp/runtime/vm/constant.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/type-alias.h"
#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/hhbbc/bc.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/src-loc.h"

namespace HPHP { namespace HHBBC { namespace php {

//////////////////////////////////////////////////////////////////////

struct ExnNode;
struct Func;
struct Unit;
struct WideFunc;

//////////////////////////////////////////////////////////////////////

struct SrcInfo {
  LineRange loc;
  LSString docComment;
};

//////////////////////////////////////////////////////////////////////

/*
 * A basic block in our factored control flow graph.
 *
 * Blocks terminate on control flow, except exceptional control flow.
 * We keep a set of "throw exits" representing all possible early
 * exits due to exceptional control flow.
 */
struct Block {
  /*
   * Instructions in the block.  Never empty guarantee.
   */
  BytecodeVec hhbcs;

  /*
   * The id of this block's ExnNode, or NoExnNodeId if there is none.
   */
  ExnNodeId exnNodeId{NoExnNodeId};

  /*
   * Edges coming out of blocks are repesented in three ways:
   *
   *  - fallthrough edges (the end of the block unconditionally jumps
   *    to the named block).  If fallthroughNS is true, this edge
   *    represents a no-surprise jump.
   *
   *  - throwExit (the edges traversed for exceptions from this block)
   *
   *  - Taken edges (these are encoded in the last instruction in hhbcs).
   *
   * For the idea behind the factored exit edge thing, see "Efficient
   * and Precise Modeling of Exceptions for the Analysis of Java
   * Programs" (http://dl.acm.org/citation.cfm?id=316171).
   */
  BlockId fallthrough{NoBlockId};
  BlockId throwExit{NoBlockId};

  union {
    uint8_t initializer{0};
    struct {
      bool catchEntry: 1;
      bool fallthroughNS: 1;
      bool multiPred: 1;
      bool multiSucc: 1;
      bool dead: 1;
    };
  };
};

//////////////////////////////////////////////////////////////////////

/*
 * Exception regions.
 *
 * Each block in the program body can have a pointer to a node in the
 * exception handler tree.  This means they are in all the "exception
 * regions" for each node in the tree down to that node.  This
 * information is used to construct exception handling regions at emit
 * time.
 *
 * The catch region is described in bytecode.specification. Note though
 * that although it's not specified there, in addition to an entry offset,
 * these regions optionally list some information about iterators if the
 * reason the region is there is to free iterator variables.
 *
 * Exceptional control flow is also represented more explicitly with
 * factored exit edges (see php::Block).  This tree structure just
 * exists to get the EHEnts right.
 */

struct CatchRegion { BlockId catchEntry;
                     Id iterId; };

struct ExnNode {
  ExnNodeId idx;
  uint32_t depth;
  CompactVector<ExnNodeId> children;
  ExnNodeId parent;
  CatchRegion region;
};

//////////////////////////////////////////////////////////////////////

/*
 * Metadata about a parameter to a php function.
 */
struct Param {
  /*
   * Default value for this parameter, or KindOfUninit if it has no
   * default value.
   */
  TypedValue defaultValue;

  /*
   * Pointer to the block we'll enter for default-value initialization
   * of this parameter, or nullptr if this parameter had no default
   * value initializer.
   */
  BlockId dvEntryPoint;

  /*
   * Information about the parameter's typehint, if any.
   *
   * NOTE: this is represented in the repo as a string type name and
   * some TypeConstraint::Flags.
   */
  TypeConstraint typeConstraint;

  /*
   * User-visible version of the type constraint as a string.
   * Propagated for reflection.
   */
  LSString userTypeConstraint;

  CompactVector<TypeConstraint> upperBounds;

  /*
   * Each parameter of a func can have arbitrary user attributes.
   */
  UserAttributeMap userAttributes;

  /*
   * Evalable php code that will give the default argument.  This is
   * redundant with the dv initializer, but gets propagated through
   * for reflection.
   */
  LSString phpCode;

  /*
   * The type of the arguments for builtin functions, or for HNI
   * functions with a native implementation.  folly::none for
   * non-builtins.
   */
  folly::Optional<DataType> builtinType;

  /*
   * Whether this parameter is passed as inout.
   */
  bool inout: 1;

  /*
   * Whether this parameter is a variadic capture.
   */
  bool isVariadic: 1;
};

/*
 * Metadata about a local variable in a function.  Name may be
 * nullptr, for unnamed locals.
 */
struct Local {
  LSString  name;
  uint32_t id         : 31;
  uint32_t killed     : 1;
  uint32_t nameId     : 31;
  uint32_t unusedName : 1;
};

/*
 * Extra information for function with a HNI native implementation.
 */
struct NativeInfo {
  /*
   * Return type from the C++ implementation function, as an optional DataType;
   * folly::none stands for a Variant return.
   */
  folly::Optional<DataType> returnType;
};

using BlockVec = CompactVector<copy_ptr<Block>>;

/*
 * Separate out the fields that need special attention when copying,
 * so that Func can just have default copy/move semantics.
 */
struct FuncBase {
  FuncBase() = default;
  FuncBase(const FuncBase&);
  FuncBase(FuncBase&&) = delete;
  FuncBase& operator=(const FuncBase&) = delete;

  /*
   * Catch regions form a tree structure.  The tree is hanging
   * off the func here, with children ids.  Each block that is
   * within a catch region has the index into this array of the
   * inner-most ExnNode protecting it.
   *
   * Note that this is updated during the concurrent analyze pass.
   */
  CompactVector<ExnNode> exnNodes;

  /*
   * For HNI-based extensions, additional information for functions
   * with a native-implementation is here.  If this isn't a function
   * with an HNI-based native implementation, this will be nullptr.
   */
  std::unique_ptr<NativeInfo> nativeInfo;

private:
  /*
   * All owning pointers to blocks are in this vector, which has the
   * blocks in an unspecified order.  Blocks use BlockIds to represent
   * control flow arcs. The id of a block is its index in this vector.
   *
   * Use WideFunc to access this data.
   */
  copy_ptr<CompactVector<char>> rawBlocks;

  friend struct WideFunc;
};

/*
 * Representation of a function, class method, or pseudomain function.
 */
struct Func : FuncBase {
  /*
   * An index, so we can lookup auxiliary structures efficiently
   */
  uint32_t idx;

  /*
   * Basic information about the function.
   */
  LSString name;
  SrcInfo srcInfo;
  Attr attrs;

  /*
   * Parameters and locals.
   *
   * There are at least as many locals as parameters (parameters are also
   * locals - the names of parameters are stored in the locals vector.)
   */
  CompactVector<Param> params;
  CompactVector<Local> locals;


  /*
   * Which unit defined this function.  If it is a method, the cls
   * field will be set to the class that contains it.
   */
  Unit* unit;
  Class* cls;

  /*
   * Entry point blocks for default value initializers.
   *
   * Note that in PHP you can declare functions where some of the
   * earlier parameters have default values, and later ones don't.  In
   * this case we'll have NoBlockIds after the first valid entry here.
   */
  CompactVector<BlockId> dvEntries;

  /*
   * The number of (nested) iterators used within this function.
   */
  IterId numIters;

  /*
   * The number of closures used within this function.
   */
  ClosureId numClosures;

  /*
   * Entry point to the function when the number of passed args is
   * equal to the number of parameters.
   */
  BlockId mainEntry;

  /*
   * User-visible return type specification as a string.  This is only
   * passed through to expose it to reflection.
   */
  LSString returnUserType;

  /*
   * If traits are being flattened by hphpc, we keep the original
   * filename of a function (the file that defined the trait) so
   * backtraces and things work correctly.  Otherwise this is nullptr.
   * Similarly, if hhbbc did the flattening itself, we need the original
   * unit, to get to the srcLocs. Once we stop flattening in hphpc, we can
   * drop the originalFilename.
   */
  LSString originalFilename;
  Unit* originalUnit{};

  /*
   * This is the generated function for a closure body.  I.e. this
   * function contains the code that should run when the closure is
   * invoked.
   */
  bool isClosureBody : 1;

  /*
   * This is an async function.
   */
  bool isAsync : 1;

  /*
   * This is a generator.
   */
  bool isGenerator : 1;

  /*
   * This generator yields key value pairs.
   */
  bool isPairGenerator : 1;

  bool isMemoizeWrapper : 1;
  bool isMemoizeWrapperLSB : 1;

  bool isMemoizeImpl : 1;

  /*
   * This is a reified function.
   */
  bool isReified : 1;

  bool noContextSensitiveAnalysis: 1;

  bool hasInOutArgs : 1;

  bool sampleDynamicCalls : 1;

  bool hasCreateCl : 1; // Function has CreateCl opcode

  /*
   * Type parameter upper bounds. May be enforced and used for optimizations.
   */
  bool hasParamsWithMultiUBs : 1;
  bool hasReturnWithMultiUBs : 1;
  CompactVector<TypeConstraint> returnUBs;

  /*
   * Return type specified in the source code (ex. "function foo(): Bar").
   * HHVM checks if the a function's return value matches it's return type
   * constraint via the VerifyRetType* instructions.
   */
  TypeConstraint retTypeConstraint;

  /*
   * User attribute list.
   */
  UserAttributeMap userAttributes;

  /*
   * Lists of all static coeffect names and coeffect rules
   */
  CompactVector<LowStringPtr> staticCoeffects;
  CompactVector<CoeffectRule> coeffectRules;
};

//////////////////////////////////////////////////////////////////////

/*
 * A class property.
 *
 * Both static and instance properties use this structure.
 */
struct Prop {
  LSString name;
  Attr attrs;
  UserAttributeMap userAttributes;
  LSString docComment;

  LSString userType;
  TypeConstraint typeConstraint;
  CompactVector<TypeConstraint> ubs;

  /*
   * The default value of the property, for properties with scalar
   * initializers.  May be KindOfUninit in some cases where the
   * property should not have an initial value (i.e. not even null).
   */
  TypedValue val;
};

/*
 * A class constant.
 */
struct Const {
  LSString name;

  // The class that defined this constant.
  php::Class* cls;

  /*
   * The value will be KindOfUninit if the class constant is defined
   * using an 86cinit method.
   *
   */
  folly::Optional<TypedValue> val;

  /*
   * We pass through eval'able php code and a string type constraint,
   * only for exposure to reflection.
   */
  LSString phpCode;
  LSString typeConstraint;

  std::vector<LowStringPtr> coeffects;

  ConstModifiers::Kind kind;
  bool isAbstract   : 1;
  bool isFromTrait  : 1;
  bool isNoOverride : 1;
};

/*
 * Similar to FuncBase - separate the fields that need special
 * attention when copying.
 */
struct ClassBase {
  ClassBase() = default;
  ClassBase(const ClassBase&);
  ClassBase(ClassBase&&) = delete;
  ClassBase& operator=(const ClassBase&) = delete;

  /*
   * Methods on the class. If there's an 86cinit, it must be last.
   */
  CompactVector<std::unique_ptr<php::Func>> methods;
};

/*
 * Representation of a php class declaration.
 */
struct Class : ClassBase {
  /*
   * Basic information about the class.
   */
  LSString name;
  SrcInfo srcInfo;
  Attr attrs;

  /*
   * The id used to reference the class within its unit
   */
  int32_t id;

  /*
   * Which unit defined this class.
   */
  Unit* unit;

  /*
   * Name of the parent class.
   */
  LSString parentName;

  /*
   * If this class represents a closure, this points to the class that
   * lexically contains the closure, if there was one.  If this class
   * doesn't represent a closure, this will be nullptr.
   *
   * The significance of this is that closures created lexically
   * inside of a class run as if they were part of that class context
   * (with regard to access checks, etc).
   */
  php::Class* closureContextCls;

  /*
   * Names of inherited interfaces.
   */
  CompactVector<LowStringPtr> interfaceNames;

  /*
   * Names of included enums.
   */
  CompactVector<LowStringPtr> includedEnumNames;

  /*
   * Names of used traits, number of declared (i.e., non-trait, non-inherited)
   * methods, trait alias/precedence rules (if any).
   *
   * This is using the exact structures from the runtime PreClass.  In
   * WholeProgram mode, we won't see these because traits will already be
   * flattened.
   */
  CompactVector<LowStringPtr> usedTraitNames;
  CompactVector<PreClass::ClassRequirement> requirements;
  CompactVector<PreClass::TraitPrecRule> traitPrecRules;
  CompactVector<PreClass::TraitAliasRule> traitAliasRules;

  /*
   * Properties defined on this class.
   */
  CompactVector<Prop> properties;

  /*
   * Constants defined on this class.
   */
  CompactVector<Const> constants;

  /*
   * User attributes for this class declaration.
   */
  UserAttributeMap userAttributes;

  /*
   * The underlying base type, if this is an enum
   */
  TypeConstraint enumBaseTy;

  /*
   * This is a reified class.
   */
  bool hasReifiedGenerics : 1;
  /*
   * This class has at least one const instance property.
   */
  bool hasConstProp : 1;
  /*
   * Dynamic construction of this class can yield at most a warning and is
   * sampled at a user defined rate.
   */
  bool sampleDynamicConstruct : 1;
};

struct Constant {
  Unit* unit;
  LSString name;
  TypedValue val;
  Attr attrs;
};

struct TypeAlias {
  Unit* unit;
  SrcInfo srcInfo;
  LSString name;
  LSString value;
  Attr attrs;
  AnnotType type;
  bool nullable;  // null is allowed; for ?Foo aliases
  UserAttributeMap userAttrs;
  Array typeStructure{ArrayData::CreateDict()};
};

//////////////////////////////////////////////////////////////////////
/*
 * A record field
 */
struct RecordField {
  LSString name;
  Attr attrs;
  LSString userType;
  LSString docComment;
  TypedValue val;
  TypeConstraint typeConstraint;
  UserAttributeMap userAttributes;
};
/*
 * Representation of a Hack record
 */
struct Record {
  Unit* unit;
  SrcInfo srcInfo;
  LSString name;
  LSString parentName;
  Attr attrs;
  int32_t id;
  UserAttributeMap userAttributes;
  CompactVector<RecordField> fields;
};

//////////////////////////////////////////////////////////////////////

/*
 * Information regarding a runtime/parse error in a unit
 */
struct FatalInfo {
  Location::Range fatalLoc;
  FatalOp fatalOp;
  std::string fatalMsg;
};

/*
 * Representation of a php file (normal compilation unit).
 */
struct Unit {
  int64_t sn{-1};
  SHA1 sha1;
  LSString filename;
  std::unique_ptr<FatalInfo> fatalInfo{nullptr};
  CompactVector<std::unique_ptr<Func>> funcs;
  CompactVector<std::unique_ptr<Class>> classes;
  CompactVector<std::unique_ptr<Record>> records;
  CompactVector<std::unique_ptr<TypeAlias>> typeAliases;
  CompactVector<std::unique_ptr<Constant>> constants;
  CompactVector<SrcLoc> srcLocs;
  UserAttributeMap metaData;
  UserAttributeMap fileAttributes;
};

/*
 * A php Program is a set of compilation units.
 */
struct Program {
  std::mutex lock;
  std::atomic<uint32_t> nextFuncId{};
  std::vector<std::unique_ptr<Unit>> units;
  std::vector<php::Func*> constInits;
};

//////////////////////////////////////////////////////////////////////

std::string show(const Func&);
std::string show(const Func&, const Block&);
std::string show(const Func&, const Bytecode& bc);
std::string show(const Class&, bool normalizeClosures = false);
std::string show(const Unit&, bool normalizeClosures = false);
std::string show(const Program&);
std::string local_string(const Func&, LocalId);

inline std::string show(const Func* f, const Bytecode& bc) {
  return show(*f, bc);
}

//////////////////////////////////////////////////////////////////////

bool check(const Func&);
bool check(const Class&);
bool check(const Unit&);
bool check(const Program&);

//////////////////////////////////////////////////////////////////////

}}}
