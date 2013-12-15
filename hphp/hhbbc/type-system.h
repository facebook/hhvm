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
 *                 |    Gen---+              ?X := X + InitNull
 *                 |     |    |
 *                Cls  Cell  Ref
 *                 |     |
 *              Cls<=c   +--------+--------+-------+-------+
 *                 |     |        |        |       |       |
 *              Cls=c   Unc       |        |      Obj     Res
 *                       |        |        |       |
 *                  +----+        |        |     Obj<=c
 *                 /     |        |        |       |
 *                /      |        |        |     Obj=c
 *             Null   InitUnc     |        |
 *             / |     / |\  \   Arr      Str
 *            /  |    /  | \  \  / \      / \
 *      Uninit  InitNull |  \  SArr CArr /  CStr
 *                       |   \   |      /
 *                       |    \ SArr=a /
 *                       |     \      /
 *                       |      \    /
 *                       |       \  /
 *                       |       SStr
 *                       |        |
 *                       |      SStr=s
 *                       |
 *                       +----------+-------+
 *                       |          |       |
 *                      Bool       Int     Dbl
 *                      /  \        |       |
 *                    True False  Int=n   Dbl=n
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
  BStr      = BSStr | BCStr,
  BArr      = BSArr | BCArr,

  // Nullable types.
  BOptTrue     = BInitNull | BTrue,
  BOptFalse    = BInitNull | BFalse,
  BOptBool     = BInitNull | BBool,
  BOptInt      = BInitNull | BInt,       // may have value
  BOptDbl      = BInitNull | BDbl,       // may have value
  BOptSStr     = BInitNull | BSStr,      // may have value
  BOptCStr     = BInitNull | BCStr,
  BOptStr      = BInitNull | BStr,
  BOptSArr     = BInitNull | BSArr,      // may have value
  BOptCArr     = BInitNull | BCArr,
  BOptArr      = BInitNull | BArr,
  BOptObj      = BInitNull | BObj,       // may have data
  BOptRes      = BInitNull | BRes,

  BInitUnc  = BInitNull | BBool | BInt | BDbl | BSStr | BSArr,
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
 */
using DObj = DCls;

//////////////////////////////////////////////////////////////////////

struct Type {
  explicit Type(trep t = BTop) : m_bits(t) {
    assert(checkInvariants());
  }

  /*
   * Exact equality or inequality of types.
   */
  bool operator==(Type o) const;
  bool operator!=(Type o) const { return !(*this == o); }

  /*
   * Subtype and strict subtype.
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
   */
  bool couldBe(Type o) const;

private:
  friend Type sval(SString);
  friend Type ival(int64_t);
  friend Type dval(double);
  friend Type aval(SArray);
  friend Type subObj(res::Class);
  friend Type objExact(res::Class);
  friend Type subCls(res::Class);
  friend Type clsExact(res::Class);
  friend DObj dobj_of(Type);
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
  bool checkInvariants() const;

private:
  trep m_bits;
  folly::Optional<Data> m_data;
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
X(Str)
X(Arr)
X(InitUnc)
X(Unc)

X(OptTrue)
X(OptFalse)
X(OptBool)
X(OptInt)
X(OptDbl)
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
 * Pre: t.strictSubtypeOf(TObj) ||
 *        (t.subtypeOf(TOptObj) && unopt(t).strictSubtypeOf(TObj))
 */
DObj dobj_of(Type t);

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

//////////////////////////////////////////////////////////////////////

}}

#endif
