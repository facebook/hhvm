/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HHBBC_REPRESENTATION_H_
#define incl_HHBBC_REPRESENTATION_H_

#include <string>
#include <tuple>
#include <vector>
#include <memory>
#include <unordered_map>
#include <list>

#include <boost/variant.hpp>

#include "hphp/util/md5.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/vm/type-alias.h"
#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/src-loc.h"
#include "hphp/hhbbc/bc.h"

namespace HPHP { namespace HHBBC {
namespace php {

//////////////////////////////////////////////////////////////////////

struct Func;
struct ExnNode;
struct Unit;

//////////////////////////////////////////////////////////////////////

struct SrcInfo {
  LineRange loc;
  SString docComment;
};

//////////////////////////////////////////////////////////////////////

/*
 * A basic block in our factored control flow graph.
 *
 * Blocks terminate on control flow, except exceptional control flow.
 * We keep a set of "factored edges" representing all possible early
 * exits due to exceptional control flow.
 */
struct Block {
  /*
   * Blocks in HHBC are each part of a bytecode "section".  The section
   * is either the "primary function body", or a fault funclet.  We
   * represent fault funclet sections with unique ids.
   *
   * Each section must be a contiguous region of bytecode, with the
   * primary function body first.  These ids are tracked just to
   * maintain this invariant at emit time.
   */
  enum class Section : uint32_t { Main = 0 };
  Section section;

  /*
   * Blocks have unique ids within a given function.
   */
  uint32_t id;

  /*
   * Instructions in the block.  Never empty guarantee.
   */
  std::vector<Bytecode> hhbcs;

  /*
   * The pointer for this block's exception region, or nullptr if
   * there is none.
   */
  borrowed_ptr<ExnNode> exnNode;

  /*
   * Edges coming out of blocks are repesented in three ways:
   *
   *  - fallthrough edges (the end of the block unconditionally jumps
   *    to the named block).  If fallthroughNS is true, this edge
   *    represents a no-surprise jump.
   *
   *  - Taken edges (these are encoded in the last instruction in hhbcs).
   *
   *  - factoredExits (these represent edges traversed for exceptions
   *    mid-block)
   *
   * For the idea behind the factored exit edge thing, see "Efficient
   * and Precise Modeling of Exceptions for the Analysis of Java
   * Programs" (http://dl.acm.org/citation.cfm?id=316171).
   */
  borrowed_ptr<Block> fallthrough;
  bool fallthroughNS = false;
  std::vector<borrowed_ptr<Block>> factoredExits;
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
 * There are two types of regions; TryRegions and FaultRegions.  These
 * correspond to the two types of regions described in
 * bytecode.specification.  Note though that although it's not
 * specified there, in addition to a fault entry offset, fault regions
 * optionally list some information about iterators if the reason the
 * fault region is there is to free iterator variables.
 *
 * Exceptional control flow is also represented more explicitly with
 * factored exit edges (see php::Block).  This tree structure just
 * exists to get the EHEnts right.
 *
 * Note: blocks in fault funclets will have factored edges to the
 * blocks listed as handlers in any ExnNode that contained the
 * fault-protected region, since those control flow paths are
 * possible.  Generally they will have nullptr for their exnNode
 * pointers, however, although they may also have other EH-protected
 * regions inside of them (this currently occurs in the case of
 * php-level finally blocks cloned into fault funclets).
 */

struct FaultRegion { borrowed_ptr<Block> faultEntry;
                     Id iterId;
                     bool itRef; };

using CatchEnt     = std::pair<const StringData*,borrowed_ptr<Block>>;
struct TryRegion   { std::vector<CatchEnt> catches; };

struct ExnNode {
  uint32_t id;

  borrowed_ptr<ExnNode> parent;
  std::vector<std::unique_ptr<ExnNode>> children;

  boost::variant<FaultRegion,TryRegion> info;
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
  Cell defaultValue;

  /*
   * Pointer to the block we'll enter for default-value initialization
   * of this parameter, or nullptr if this parameter had no default
   * value initializer.
   */
  borrowed_ptr<php::Block> dvEntryPoint;

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
  SString userTypeConstraint;

  /*
   * Evalable php code that will give the default argument.  This is
   * redundant with the dv initializer, but gets propagated through
   * for reflection.
   */
  SString phpCode;

  /*
   * Each parameter of a func can have arbitrary user attributes.
   */
  UserAttributeMap userAttributes;

  /*
   * Whether this parameter is passed by reference.
   */
  bool byRef : 1;
};

/*
 * Metadata about a local variable in a function.  Name may be
 * nullptr, for unnamed locals.
 */
struct Local {
  SString name;
  uint32_t id;
};

/*
 * Metadata about function iterator variables.
 */
struct Iter {
  uint32_t id;
};

/*
 * Static local information.  For each static local, we need to keep
 * the php code around for reflection.
 */
struct StaticLocalInfo {
  SString name;
  SString phpCode;
};

/*
 * Representation of a function, class method, or pseudomain function.
 */
struct Func {
  /*
   * Basic information about the function.
   */
  SString name;
  SrcInfo srcInfo;
  Attr attrs;

  /*
   * Which unit defined this function.  If it is a method, the cls
   * field will be set to the class that contains it.
   */
  borrowed_ptr<Unit> unit;
  borrowed_ptr<Class> cls;

  /*
   * Parameters, locals, and iterators.
   *
   * There are at least as many locals as parameters (parameters are
   * also locals---the names of parameters are stored in the locals
   * vector).
   */
  std::vector<Param> params;
  std::vector<std::unique_ptr<Local>> locals;
  std::vector<std::unique_ptr<Iter>> iters;
  std::vector<StaticLocalInfo> staticLocals;

  /*
   * Whether or not this function is a top-level function.  (Defined
   * outside of any other function body.)
   */
  bool top : 1;

  /*
   * This is the generated function for a closure body.  I.e. this
   * function contains the code that should run when the closure is
   * invoked.
   */
  bool isClosureBody : 1;

  /*
   * This is the "generator body" function for a continuation.
   * I.e. this is the code that runs when a generator is asked to
   * yield a new element.
   */
  bool isGeneratorBody : 1;

  /*
   * This is the generator body for a generator closure.  (I.e. a
   * closure with yield statements in it.)
   */
  bool isGeneratorFromClosure : 1;

  /*
   * This generator yields key value pairs.
   */
  bool isPairGenerator : 1;

  /*
   * This is an async function.  This flag is set on both the "inner"
   * and "outer" functions for a generator when it used async/await
   * instead of yield.
   */
  bool isAsync : 1;

  /*
   * All owning pointers to blocks are in this vector, which has the
   * blocks in an unspecified order.  Blocks have borrowed pointers to
   * each other to represent control flow arcs.
   */
  std::vector<std::unique_ptr<Block>> blocks;

  /*
   * Greatest block id in the function plus one.
   */
  uint32_t nextBlockId;

  /*
   * Try and fault regions form a tree structure.  The tree is hanging
   * off the func here, with children pointers.  Each block that is
   * within a try or fault region has a pointer to the inner-most
   * ExnNode protecting it.
   */
  std::vector<std::unique_ptr<ExnNode>> exnNodes;

  /*
   * Entry point blocks for default value initializers.
   *
   * Note that in PHP you can declare functions where some of the
   * earlier parameters have default values, and later ones don't.  In
   * this case we'll have nulls after the first non-null entry here.
   */
  std::vector<borrowed_ptr<Block>> dvEntries;

  /*
   * Entry point to the function when the number of passed args is
   * equal to the number of parameters.
   */
  borrowed_ptr<Block> mainEntry;

  /*
   * User attribute list.
   */
  UserAttributeMap userAttributes;

  /*
   * This is the name of "inner" function for a generator.  When compiling
   * a generator, we generate two funtions.  The "outer" one stores (with
   * this field set) allocates the continuation object and returns it.  The
   * "inner" one named here will have isGeneratorBody set.
   */
  SString generatorBodyName;

  /*
   * User-visible return type specification as a string.  This is only
   * passed through to expose it to reflection.
   */
  SString returnUserType;

  /*
   * If traits are being flattened by hphpc, we keep the original
   * filename of a function (the file that defined the trait) so
   * backtraces and things work correctly.  Otherwise this is nullptr.
   */
  SString originalFilename;
};

//////////////////////////////////////////////////////////////////////

/*
 * A class property.
 *
 * Both static and instance properties use this structure.
 */
struct Prop {
  SString name;
  Attr attrs;
  SString docComment;

  /*
   * Properties can have string type constraints, which we need to
   * propagate through just for reflection purposes.
   */
  SString typeConstraint;

  /*
   * The default value of the property, for properties with scalar
   * initializers.  May be KindOfUninit in some cases where the
   * property should not have an initial value (i.e. not even null).
   */
  Cell val;
};

/*
 * A class constant.
 */
struct Const {
  SString name;
  Cell val;

  /*
   * We pass through eval'able php code and a string type constraint,
   * only for exposure to reflection.
   */
  SString phpCode;
  SString typeConstraint;
};

/*
 * Representation of a php class declaration.
 */
struct Class {
  /*
   * Basic information about the class.
   */
  SString name;
  SrcInfo srcInfo;
  Attr attrs;

  /*
   * Which unit defined this class.
   */
  borrowed_ptr<Unit> unit;

  /*
   * Hoistability of this class.  See the description in class.h
   * formation on hoistability.
   */
  PreClass::Hoistable hoistability;

  /*
   * The function that contains the DefCls for this class, or nullptr
   * in the case of classes that do not have a DefCls (e.g. closure
   * classes).
   *
   * The plan was to use this information in part to compute
   * hoistability at emit time, but it's unused right now.  (Using
   * hphpc's hoistability notions.)
   */
  // borrowed_ptr<php::Func> definingFunc;

  /*
   * Name of the parent class.
   */
  SString parentName;

  /*
   * Names of inherited interfaces.
   */
  std::vector<SString> interfaceNames;

  /*
   * Names of used traits, and the trait alias/precedence rules (if
   * any).
   *
   * This is using the exact structures from the runtime PreClass.  In
   * WholeProgram mode, we won't see these because traits will already
   * be flattened.
   */
  std::vector<SString> usedTraitNames;
  std::vector<PreClass::TraitRequirement> traitRequirements;
  std::vector<PreClass::TraitPrecRule> traitPrecRules;
  std::vector<PreClass::TraitAliasRule> traitAliasRules;

  /*
   * Methods on the class.
   */
  std::vector<std::unique_ptr<php::Func>> methods;

  /*
   * Properties defined on this class.
   */
  std::vector<Prop> properties;

  /*
   * Constants defined on this class.
   */
  std::vector<Const> constants;

  /*
   * User attributes for this class declaration.
   */
  UserAttributeMap userAttributes;
};

//////////////////////////////////////////////////////////////////////

using TypeAlias = ::HPHP::TypeAlias;

//////////////////////////////////////////////////////////////////////

/*
 * Representation of a php file (normal compilation unit).
 */
struct Unit {
  MD5 md5;
  SString filename;
  std::unique_ptr<Func> pseudomain;
  std::vector<std::unique_ptr<Func>> funcs;
  std::vector<std::unique_ptr<Class>> classes;
  std::vector<std::unique_ptr<TypeAlias>> typeAliases;
};

/*
 * A php Program is a set of compilation units.
 */
struct Program {
  std::vector<std::unique_ptr<Unit>> units;
};

//////////////////////////////////////////////////////////////////////

std::string show(const Func&);
std::string show(const Class&);
std::string show(const Unit&);
std::string show(const Program&);

//////////////////////////////////////////////////////////////////////

bool check(const Func&);
bool check(const Class&);
bool check(const Unit&);
bool check(const Program&);

//////////////////////////////////////////////////////////////////////

}}}

#endif
