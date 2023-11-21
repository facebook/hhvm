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

namespace HPHP {
namespace HHBBC {

//////////////////////////////////////////////////////////////////////

struct Index;

namespace php {

//////////////////////////////////////////////////////////////////////

struct ExnNode;
struct Func;
struct Unit;
struct WideFunc;

//////////////////////////////////////////////////////////////////////

struct SrcInfo {
  LineRange loc;
  LSString docComment;

  template <typename SerDe> void serde(SerDe&);
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
   *    to the named block).
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
      bool multiPred: 1;
      bool multiSucc: 1;
      bool dead: 1;
    };
  };

  template <typename SerDe> void serde(SerDe&);
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

struct CatchRegion {
  BlockId catchEntry;
  Id iterId;
  template <typename SerDe> void serde(SerDe&);
};

struct ExnNode {
  ExnNodeId idx;
  uint32_t depth;
  CompactVector<ExnNodeId> children;
  ExnNodeId parent;
  CatchRegion region;
  template <typename SerDe> void serde(SerDe&);
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

  TypeIntersectionConstraint upperBounds;

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
   * Whether this parameter is passed as inout.
   */
  bool inout: 1;

  /*
   * Whether this parameter is passed as readonly.
   */
  bool readonly: 1;

  /*
   * Whether this parameter is a variadic capture.
   */
  bool isVariadic: 1;

  template <typename SerDe> void serde(SerDe&);
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

  template <typename SerDe> void serde(SerDe&);
};

using BlockVec = CompactVector<copy_ptr<Block>>;

using CompressedBytecode = CompactVector<char>;
using CompressedBytecodePtr = copy_ptr<CompactVector<char>>;

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
   * Does this function have a native (C++) implementation?
   */
  bool isNative;

  /*
   * All owning pointers to blocks are in this vector, which has the
   * blocks in an unspecified order.  Blocks use BlockIds to represent
   * control flow arcs. The id of a block is its index in this vector.
   *
   * Use WideFunc to access this data.
   */
  CompressedBytecodePtr rawBlocks;
};

/*
 * Representation of a function or class method.
 */
struct Func : FuncBase {
  /*
  * An index, so we can lookup auxiliary structures efficiently. This
  * field is not serialized and its meaning is context dependent.
  */
  uint32_t idx{std::numeric_limits<uint32_t>::max()};

  /*
   * If this function is a method, it's index in the owning Class'
   * methods table (2^32-1 otherwise so that misuse will tend to cause
   * crashes).
   */
  uint32_t clsIdx{std::numeric_limits<uint32_t>::max()};

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
  LSString unit;
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
  LSString originalUnit{};

  /*
   * The reference of the trait where the method was originally
   * defined.  This is used to detected if a method is imported
   * multiple times via different use-chains as the pair (name,
   * originalClass) uniquely identifies a method. If nullptr, the
   * "original" class is just the function's class.
   */
  LSString originalClass{};

  /*
   * The module where the method was initially defined, irrespective
   * of trait inlining.  If the requiresFromOriginalModule flag is set
   * then this field is exported to HHVM to implement the
   * Module Level Trait semantics.
   */
  LSString originalModuleName{};

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

  bool isReadonlyReturn : 1;

  bool isReadonlyThis : 1;

  /*
   * Type parameter upper bounds. May be enforced and used for optimizations.
   */
  bool hasParamsWithMultiUBs : 1;
  bool hasReturnWithMultiUBs : 1;

  /*
   * Method was originally declared in a trait with Module Level Trait semantics
   * (e.g. the <<__ModuleLevelTrait>> attribute was specified on the original trait).
   */
  bool fromModuleLevelTrait : 1;
  bool requiresFromOriginalModule : 1;

  TypeIntersectionConstraint returnUBs;

  /*
   * Return type specified in the source code (ex. "function foo(): Bar").
   * HHVM checks if the a function's return value matches it's return type
   * constraint via the VerifyRetType* instructions.
   */
  TypeConstraint retTypeConstraint;

  /*
   * Static coeffects in bit encoding
   */
  RuntimeCoeffects requiredCoeffects{RuntimeCoeffects::none()};
  RuntimeCoeffects coeffectEscapes{RuntimeCoeffects::none()};

  /*
   * Lists of all static coeffect names and coeffect rules
   */
  CompactVector<LowStringPtr> staticCoeffects;
  CompactVector<CoeffectRule> coeffectRules;

  /*
   * User attribute list.
   */
  UserAttributeMap userAttributes;

  template <typename SerDe> void serde(SerDe&, Class* c = nullptr);
};

/*
 * Bytecode for a function (global function or class method). While
 * using extern-worker, this is stored separately and not within
 * php::Func.
 */
struct FuncBytecode {
  FuncBytecode() = default;
  FuncBytecode(SString name, CompressedBytecodePtr bc)
    : name{name}, bc{std::move(bc)} {}

  LSString name;
  CompressedBytecodePtr bc;

  /*
   * Bytecode is stored using copy_ptr, which allows multiple
   * functions to share the same bytecode if they're the same. This
   * gets lost if we serde the function, which can lead to increased
   * memory pressure.
   *
   * If s_reuser is non-nullptr during deserializing, it will be used
   * to try to find similar bytecode and reuse it, restoring the
   * sharing of copy_ptr (and maybe more).
   */
  using Reuser = folly_concurrent_hash_map_simd<SHA1, CompressedBytecodePtr>;
  static Reuser* s_reuser;

  template <typename SerDe> void serde(SerDe&);
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
  TypeIntersectionConstraint ubs;

  /*
   * The default value of the property, for properties with scalar
   * initializers.  May be KindOfUninit in some cases where the
   * property should not have an initial value (i.e. not even null).
   */
  TypedValue val;

  template <typename SerDe> void serde(SerDe&);
};

/*
 * A class constant.
 */
struct Const {
  LSString name;

  // The class that defined this constant.
  LSString cls;

  /*
   * The value will be KindOfUninit if the class constant is defined
   * using an 86cinit method.
   *
   */
  Optional<TypedValue> val;

  std::vector<LowStringPtr> coeffects;

  SArray resolvedTypeStructure;

  ConstModifiers::Kind kind;

  using Invariance = PreClass::Const::Invariance;
  Invariance invariance : 2;
  bool isAbstract   : 1;
  bool isFromTrait  : 1;

  template <typename SerDe> void serde(SerDe&);
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
   * Which unit defined this class.
   */
  LSString unit;

  /*
   * Which module this class was defined in.
   */
  LSString moduleName;

  /*
   * Name of the parent class.
   */
  LSString parentName;

  /*
   * If this class represents a closure, this points to the name of
   * the class that lexically contains the closure, if there was one.
   * If this class doesn't represent a closure, this will be nullptr.
   *
   * The significance of this is that closures created lexically
   * inside of a class run as if they were part of that class context
   * (with regard to access checks, etc).
   */
  LSString closureContextCls;

  /*
   * If this class represents a closure defined in a top level
   * function (not a method), this points to the name of that
   * function. Nullptr otherwise. (For closures defined within
  * classes, use closureContextCls).
   */
  LSString closureDeclFunc;

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

  template <typename SerDe> void serde(SerDe&);
};

struct ClassBytecode {
  ClassBytecode() = default;
  explicit ClassBytecode(SString cls) : cls{cls} {}
  SString cls;
  CompactVector<FuncBytecode> methodBCs;
  template <typename SerDe> void serde(SerDe&);
};

struct Constant {
  LSString name;
  TypedValue val;
  Attr attrs;

  template <typename SerDe> void serde(SerDe&);
};

struct Module {
  LSString name;
  SrcInfo srcInfo;
  Attr attrs;
  UserAttributeMap userAttributes;
  Optional<HPHP::Module::RuleSet> exports;
  Optional<HPHP::Module::RuleSet> imports;

  template <typename SerDe> void serde(SerDe&);
};

struct TypeAlias {
  SrcInfo srcInfo;
  LSString name;
  Attr attrs;
  TypeConstraint value;
  bool caseType : 1;
  UserAttributeMap userAttrs;
  Array typeStructure{ArrayData::CreateDict()};
  Array resolvedTypeStructure;

  template <typename SerDe> void serde(SerDe&);
};

//////////////////////////////////////////////////////////////////////

/*
 * Information regarding a runtime/parse error in a unit
 */
struct FatalInfo {
  // If fatalLoc is missing, this represents a verifier failure
  Optional<Location::Range> fatalLoc;
  FatalOp fatalOp;
  std::string fatalMsg;

  template <typename SerDe> void serde(SerDe& sd);
};

/*
 * Representation of a php file (normal compilation unit).
 */
struct Unit {
  LSString filename{nullptr};
  std::unique_ptr<FatalInfo> fatalInfo{nullptr};
  CompactVector<SString> funcs;
  CompactVector<SString> classes;
  CompactVector<std::unique_ptr<TypeAlias>> typeAliases;
  CompactVector<std::unique_ptr<Constant>> constants;
  CompactVector<std::unique_ptr<Module>> modules;
  CompactVector<SrcLoc> srcLocs;
  UserAttributeMap metaData;
  UserAttributeMap fileAttributes;
  LSString moduleName;
  LSString extName;
  PackageInfo packageInfo;

  template <typename SerDe> void serde(SerDe& sd);
};

/*
 * A php Program is a set of compilation units.
 */
struct Program {
  std::vector<std::unique_ptr<Func>> funcs;
  std::vector<std::unique_ptr<Class>> classes;
  std::vector<std::unique_ptr<Unit>> units;
};

//////////////////////////////////////////////////////////////////////

std::string show(const Func&);
std::string show(const Func&, const Block&);
std::string show(const Func&, const Bytecode& bc);
std::string show(const Class&);
std::string show(const Unit&,
                 const std::vector<const Class*>&,
                 const std::vector<const Func*>&);
std::string show(const Unit&, const Index&);
std::string show(const Unit&, const Program&);
std::string show(const Program&, const Index&);
std::string local_string(const Func&, LocalId);

//////////////////////////////////////////////////////////////////////

bool check(const Func&);
bool check(const Class&);
bool check(const Unit&, const Index&);
bool check(const Program&);

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

using FuncOrCls = Either<const php::Func*, const php::Class*>;

struct FuncOrClsHasher {
  size_t operator()(FuncOrCls f) const { return f.toOpaque(); }
};

std::string show(FuncOrCls);

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

MAKE_COPY_PTR_BLOB_SERDE_HELPER(HHBBC::php::Block)
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::php::Unit)
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::php::Func)
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::php::FuncBytecode)
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::php::Class)
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::php::ClassBytecode)
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::php::Constant)
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::php::TypeAlias)
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::php::Module)
MAKE_UNIQUE_PTR_BLOB_SERDE_HELPER(HHBBC::php::FatalInfo)

//////////////////////////////////////////////////////////////////////

}
