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
#ifndef incl_HHBBC_TYPE_SYSTEM_H_
#define incl_HHBBC_TYPE_SYSTEM_H_

#include <cstdint>

#include "folly/Optional.h"

#include "hphp/runtime/base/complex-types.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/index.h"

namespace HPHP { struct TypeConstraint; }
namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * Type system.
 *
 * Here's an unmaintainable ascii-art diagram:
 *
 *                      Top
 *                       |
 *                 +-----+              InitGen :=  Gen - Uninit
 *                 |     |             InitCell := Cell - Uninit
 *                Cls   Gen---+              ?X := X + InitNull
 *                 |     |    |
 *              Cls<=c  Cell  Ref
 *                 |     |
 *              Cls=c    +-------------+--------+-------+-------+
 *                       |             |        |       |       |
 *                      Unc            |        |      Obj     Res
 *                       | \           |        |      /  \
 *                       |  \          |        |  Obj<=c Obj<=WaitHandle
 *                     Prim  \         |        |    |       |
 *                     / |   InitUnc   |        |  Obj=c   WaitH<T>
 *                    /  |   /  | |    |        |
 *                   /   |  /   | |    |        |
 *                  /    | /    | |    |        |
 *                 /     |/     | |    |        |
 *              Null  InitPrim  | \    |        |
 *             /  |    / |      |  \  Arr      Str
 *            /   |   /  |      |   \ / \      / \
 *      Uninit  InitNull |      |   SArr CArr /  CStr
 *                       |      |     |      /
 *                       |      |    SArr=a /
 *                       |      |          /
 *                       |      \         /
 *                       |       \       /
 *                       |        \     /
 *                       |         \   /
 *                       |          SStr
 *                       |           |
 *                       |         SStr=s
 *                       |
 *                       +----------+
 *                       |          |
 *                      Bool       Num
 *                      /  \       |  \
 *                   True  False  Int  Dbl
 *                                 |    |
 *                               Int=n Dbl=n
 *
 * Some description of the types here:
 *
 *   {Init,}Prim
 *
 *       "Primitive" types---these can be represented in a TypedValue
 *       without a pointer to the heap.
 *
 *   {Init,}Unc
 *
 *       "Uncounted" types---values of these types don't require
 *       reference counting.
 *
 *   WaitH<T>
 *
 *       A WaitHandle that is known will either return a value of type
 *       T from its join() method (or AsyncAwait), or else throw an
 *       exception.
 *
 */

//////////////////////////////////////////////////////////////////////

enum trep : uint32_t {
  BBottom   = 0,

  BUninit   = 1 << 0,
  BInitNull = 1 << 1,
  BFalse    = 1 << 2,
  BTrue     = 1 << 3,
  BInt      = 1 << 4,
  BDbl      = 1 << 5,
  BSStr     = 1 << 6, // static string
  BCStr     = 1 << 7, // counted string
  BSArr     = 1 << 8, // static array
  BCArr     = 1 << 9, // counted array
  BObj      = 1 << 10,
  BRes      = 1 << 11,
  BCls      = 1 << 12,
  BRef      = 1 << 13,

  BNull     = BUninit | BInitNull,
  BBool     = BFalse | BTrue,
  BNum      = BInt | BDbl,
  BStr      = BSStr | BCStr,
  BArr      = BSArr | BCArr,

  // Nullable types.
  BOptTrue     = BInitNull | BTrue,
  BOptFalse    = BInitNull | BFalse,
  BOptBool     = BInitNull | BBool,
  BOptInt      = BInitNull | BInt,       // may have value
  BOptDbl      = BInitNull | BDbl,       // may have value
  BOptNum      = BInitNull | BNum,
  BOptSStr     = BInitNull | BSStr,      // may have value
  BOptCStr     = BInitNull | BCStr,
  BOptStr      = BInitNull | BStr,
  BOptSArr     = BInitNull | BSArr,      // may have value
  BOptCArr     = BInitNull | BCArr,
  BOptArr      = BInitNull | BArr,
  BOptObj      = BInitNull | BObj,       // may have data
  BOptRes      = BInitNull | BRes,

  BInitPrim = BInitNull | BBool | BNum,
  BPrim     = BInitPrim | BUninit,
  BInitUnc  = BInitPrim | BSStr | BSArr,
  BUnc      = BInitUnc | BUninit,
  BInitCell = BInitNull | BBool | BInt | BDbl | BStr | BArr | BObj | BRes,
  BCell     = BUninit | BInitCell,
  BInitGen  = BInitCell | BRef,
  BGen      = BUninit | BInitGen,

  BTop      = static_cast<uint32_t>(-1),
};

//////////////////////////////////////////////////////////////////////

/*
 * Information about a class type.  The class is either exact or a
 * subtype of the supplied class.
 */
struct DCls {
  enum { Exact, Sub } type;
  res::Class cls;
};

/*
 * Information about a specific object type.  The class is either
 * exact or a subtype of the supplied class.
 *
 * If the class is WaitHandle, we can also carry a type that joining
 * the wait handle will produce.  (This is hoisted into Type to keep
 * DObj and Type::Data trivially copyable for now.)
 */
struct DObj {
  enum { Exact, Sub } type;
  res::Class cls;
};

//////////////////////////////////////////////////////////////////////

struct Type {
  Type() : m_bits(BTop) {
    assert(checkInvariants());
  }
  explicit Type(trep t) : m_bits(t) {
    assert(checkInvariants());
  }

  /*
   * Exact equality or inequality of types.
   */
  bool operator==(Type o) const;
  bool operator!=(Type o) const { return !(*this == o); }

  /*
   * Returns true if this type is definitely going to be a subtype or a strict
   * subtype of `o' at runtime.  If this function returns false, this may
   * still be a subtype of `o' at runtime, it just may not be known.
   */
  bool subtypeOf(Type o) const;
  bool strictSubtypeOf(Type o) const;

  /*
   * Subtype of any of the list of types.
   */
  template<class... Types>
  bool subtypeOfAny(Type t, Types... ts) const {
    return subtypeOf(t) || subtypeOfAny(ts...);
  }
  bool subtypeOfAny() const { return false; }

  /*
   * Returns whether there are any values of this type that are also
   * values of the type `o'.
   * When this function returns false, it is known that this type
   * must not be in any subtype relationship with the argument Type 'o'.
   * When true is returned the two types may still be unrelated but it is
   * not possible to tell.
   * Essentially this function can conservatively return true but must be
   * precise when returning false.
   */
  bool couldBe(Type o) const;

private:
  friend Type wait_handle(const Index&, Type);
  friend bool is_specialized_wait_handle(const Type&);
  friend Type wait_handle_inner(const Type&);
  friend Type sval(SString);
  friend Type ival(int64_t);
  friend Type dval(double);
  friend Type aval(SArray);
  friend Type subObj(res::Class);
  friend Type objExact(res::Class);
  friend Type subCls(res::Class);
  friend Type clsExact(res::Class);
  friend DObj dobj_of(const Type&);
  friend DCls dcls_of(Type);
  friend Type union_of(Type, Type);
  friend Type opt(Type);
  friend Type unopt(Type);
  friend bool is_opt(Type);
  friend folly::Optional<Cell> tv(Type);
  friend std::string show(Type);

private:
  union Data {
    Data() : sval(nullptr) {}

    SString sval;
    int64_t ival;
    double  dval;
    SArray  aval;
    DObj    dobj;
    DCls    dcls;
  };

private:
  Type(trep t, Data d);
  bool equivData(Type o) const;
  bool subtypeData(Type o) const;
  bool couldBeData(Type o) const;
  bool checkInvariants() const;

private:
  trep m_bits;
  folly::Optional<Data> m_data;
  copy_ptr<Type> m_whType;
};

#define X(y) const Type T##y = Type(B##y);

X(Bottom)

X(Uninit)
X(InitNull)
X(False)
X(True)
X(Int)
X(Dbl)
X(SStr)
X(CStr)
X(SArr)
X(CArr)
X(Obj)
X(Res)
X(Cls)
X(Ref)

X(Null)
X(Bool)
X(Num)
X(Str)
X(Arr)
X(InitPrim)
X(Prim)
X(InitUnc)
X(Unc)

X(OptTrue)
X(OptFalse)
X(OptBool)
X(OptInt)
X(OptDbl)
X(OptNum)
X(OptSStr)
X(OptCStr)
X(OptStr)
X(OptSArr)
X(OptCArr)
X(OptArr)
X(OptObj)
X(OptRes)

X(InitCell)
X(Cell)
X(InitGen)
X(Gen)

X(Top)

#undef X

//////////////////////////////////////////////////////////////////////

/*
 * Return WaitH<T> for a type t.
 */
Type wait_handle(const Index&, Type t);

/*
 * Return T from a WaitH<T>.
 *
 * Pre: is_specialized_handle(t);
 */
Type wait_handle_inner(const Type& t);

/*
 * Create Types that represent constant values.
 */
Type sval(SString);
Type ival(int64_t);
Type dval(double);
Type aval(SArray);

/*
 * Create types for objects or classes with some known constraint on
 * which res::Class is associated with them.
 */
Type subObj(res::Class);
Type objExact(res::Class);
Type subCls(res::Class);
Type clsExact(res::Class);

/*
 * Create the optional version of the Type t.
 *
 * Pre: there must be an optional version of the type t.
 */
Type opt(Type t);

/*
 * Return the non-optional version of the Type t.
 *
 * Pre: is_opt(t)
 */
Type unopt(Type t);

/*
 * Returns whether a given type is a subtype of one of the predefined
 * optional types.  (Note that this does not include types like
 * TInitUnc---it's only the TOpt* types.)
 */
bool is_opt(Type t);

/*
 * Returns true if type 't' represents a "specialized" object, that is
 * an object of a known class, or an optional object of a known class.
 */
bool is_specialized_obj(Type t);

/*
 * Returns whether `t' is a WaitH<T> or ?WaitH<T> for some T.
 *
 * Note that this function returns false for Obj<=WaitHandle with no
 * tracked inner type.
 */
bool is_specialized_wait_handle(const Type& t);

/*
 * Returns the best known TCls subtype for an object type.
 *
 * Pre: t.subtypeOf(TObj)
 */
Type objcls(Type t);

/*
 * If the type t has a known constant value, return it as a Cell.
 * Otherwise return folly::none.
 *
 * The returned Cell can only contain non-reference-counted types.
 */
folly::Optional<Cell> tv(Type t);

/*
 * Get the type in our typesystem that corresponds to an hhbc
 * IsTypeOp.
 *
 * Pre: op != IsTypeOp::Scalar
 */
Type type_of_istype(IsTypeOp op);

/*
 * Return the DObj structure for a strict subtype of TObj or TOptObj.
 *
 * Pre: is_specialized_obj(t)
 */
DObj dobj_of(const Type& t);

/*
 * Return the DCls structure for a strict subtype of TCls.
 *
 * Pre: t.strictSubtypeOf(TCls)
 */
DCls dcls_of(Type t);

/*
 * Create a Type from a Cell.
 *
 * Pre: the cell must contain a non-reference-counted type.
 * Post: returned type is a subtype of TUnc
 */
Type from_cell(Cell tv);

/*
 * Create a Type from a DataType.  KindOfString and KindOfStaticString
 * are both treated as TStr.
 *
 * Pre: dt is one of the DataTypes that actually represent php values
 * (or KindOfUninit).
 */
Type from_DataType(DataType dt);

/*
 * Create a Type from a builtin type specification string.
 *
 * This is used for HNI class properties.  We assume that these are
 * accurate.  s may be null.
 */
Type from_hni_constraint(SString s);

/*
 * Make a type that represents values from either of the supplied
 * types.
 */
Type union_of(Type a, Type b);

/*
 * Returns the smallest type that `a' is a subtype of, from the
 * following set: TGen, TInitCell, TRef, TUninit, TCls.
 *
 * Pre: `a' is a subtype of TGen, or TCls.
 */
Type stack_flav(Type a);

/*
 * Force any type that contains SStr and SArr to contain Arr and Str.
 * This is needed for some operations that can change static arrays or
 * strings into non-static ones.  Doesn't change the type if it can't
 * contain SStr or SArr.
 */
Type loosen_statics(Type);

/*
 * Force any type that corresponds to a constant php value to contain
 * all values of that php type.
 *
 * Precisely: strict subtypes of TInt, TDbl, TBool, TSStr, and TSArr
 * become exactly that corresponding type.  Additionally, TOptTrue and
 * TOptFalse become TOptBool.  All other types are unchanged.
 */
Type loosen_values(Type);

/*
 * If t contains TUninit, returns the best type we can that contains
 * at least everything t contains, but doesn't contain TUninit.  Note
 * that this function will return TBottom for TUninit.
 *
 * Pre: t.subtypeOf(TCell)
 */
Type remove_uninit(Type t);

//////////////////////////////////////////////////////////////////////

}}

#endif
