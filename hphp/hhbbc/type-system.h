/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <vector>
#include <utility>

#include <boost/container/flat_map.hpp>

#include "folly/Optional.h"

#include "hphp/runtime/base/complex-types.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/index.h"

namespace HPHP { namespace HHBBC {

struct Type;

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
 *                    /  |   /  |  |   |        |
 *                   /   |  /   |  |   |        |
 *                  /    | /    |  |   |        |
 *                 /     |/     |  |   |        |
 *              Null  InitPrim  |  |   |        |
 *             /  |    / |      |  |  Arr      Str
 *            /   |   /  |      |  |  / \      / \
 *      Uninit  InitNull |      | SArr  ...   /  CStr
 *                       |      |  |         /
 *                       |      | ...       /
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
 * Some notes on some of the basic types:
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
 * Array types:
 *
 *   Arrays are divided along two dimensions: counted or uncounted,
 *   and empty or non-empty.  Unions of either are allowed.  The
 *   naming convention is {S,C,}Arr{N,E,}, where leaving out either
 *   bit means it's unknown along that dimension.  All arrays are
 *   subtypes of the Arr type.  The lattice here looks like this:
 *
 *                         Arr
 *                          |
 *                  +----+--+--+---+
 *                  |    |     |   |
 *                  |  SArr   CArr |
 *                  |   |      |   |
 *              +-------+---------------+
 *              |   |          |   |    |
 *              |  ArrN  +-----+  ArrE  |
 *              | /   \  |     | /   \  |
 *             SArrN  CArrN   CArrE  SArrE
 *
 *   "Specialized" array types may be found as subtypes of any of the
 *   above types except SArrE and CArrE, or of optional versions of
 *   the above types (e.g. as a subtype of ?ArrN).  The information
 *   about additional structure is dispayed in parenthesis, and
 *   probably best explained by some examples:
 *
 *     SArrN(Bool,Int)
 *
 *         Tuple-like static two-element array, with integer keys 0
 *         and 1, containing a Bool and an Int with unknown values.
 *
 *     Arr(Int,Dbl)
 *
 *         An array of unknown countedness that is either empty, or a
 *         tuple-like array with two elements of types Int and Dbl.
 *
 *     CArrN([Bool])
 *
 *         Non-empty reference counted array with contiguous
 *         zero-based integer keys, unknown size, values all are
 *         subtypes of Bool.
 *
 *     ArrN(x:Int,y:Int)
 *
 *         Struct-like array with known fields "x" and "y" that have
 *         Int values, and no other fields.  Struct-like arrays always
 *         have known string keys, and the type contains only those
 *         array values with exactly the given key set.  The order of
 *         the array elements is not tracked in this type system.
 *
 *     Arr([SStr:InitCell])
 *
 *         Possibly empty map-like array with unknown keys, but all
 *         non-reference counted strings, all values InitCell.  In
 *         this case the array itself may or may not be static.
 *
 *         Note that struct-like arrays will be subtypes of map-like
 *         arrays with string keys.
 *
 *     ArrN([Int:InitPrim])
 *
 *         Map-like array with only integer keys (not-necessarily
 *         contiguous) and values that are all subtypes of InitPrim.
 *         Note that the keys *may* be contiguous integers, so for
 *         example Arr([InitPrim]) <: Arr([Int => InitPrim]).
 *
 *     Arr([InitCell:InitCell])
 *
 *         Map-like array with either integer or string keys, and
 *         InitCell values, or empty.  Essentially this is the most
 *         generic array that can't contain php references.
 *
 *  TODO(#3774082): we should have a Str|Int type.
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
  BSStr     = 1 << 6,  // static string
  BCStr     = 1 << 7,  // counted string
  BSArrE    = 1 << 8,  // static empty array
  BCArrE    = 1 << 9,  // counted empty array
  BSArrN    = 1 << 10, // static non-empty array
  BCArrN    = 1 << 11, // counted non-empty array
  BObj      = 1 << 12,
  BRes      = 1 << 13,
  BCls      = 1 << 14,
  BRef      = 1 << 15,

  BNull     = BUninit | BInitNull,
  BBool     = BFalse | BTrue,
  BNum      = BInt | BDbl,
  BStr      = BSStr | BCStr,
  BSArr     = BSArrE | BSArrN,
  BCArr     = BCArrE | BCArrN,
  BArrE     = BSArrE | BCArrE,
  BArrN     = BSArrN | BCArrN,   // may have value / data
  BArr      = BArrE | BArrN,

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
  BOptSArrE    = BInitNull | BSArrE,
  BOptCArrE    = BInitNull | BCArrE,
  BOptSArrN    = BInitNull | BSArrN,     // may have value / data
  BOptCArrN    = BInitNull | BCArrN,     // may have value / data
  BOptSArr     = BInitNull | BSArr,      // may have value / data
  BOptCArr     = BInitNull | BCArr,      // may have value / data
  BOptArrE     = BInitNull | BArrE,      // may have value / data
  BOptArrN     = BInitNull | BArrN,      // may have value / data
  BOptArr      = BInitNull | BArr,       // may have value / data
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

// Tag for what kind of specialized data a Type object has.
enum class DataTag : uint8_t {
  None,
  Str,
  Obj,
  Int,
  Dbl,
  Cls,
  ArrVal,
  ArrPacked,
  ArrPackedN,
  ArrStruct,
  ArrMapN,
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
 * the wait handle will produce.
 */
struct DObj {
  enum Tag { Exact, Sub };

  explicit DObj(Tag type, res::Class cls)
    : type(type)
    , cls(cls)
  {}

  Tag type;
  res::Class cls;
  copy_ptr<Type> whType;
};

struct DArrPacked;
struct DArrPackedN;
struct DArrStruct;
struct DArrMapN;
using StructMap = boost::container::flat_map<SString,Type>;

//////////////////////////////////////////////////////////////////////

struct Type {
  Type() : m_bits(BTop) {
    assert(checkInvariants());
  }
  explicit Type(trep t) : m_bits(t) {
    assert(checkInvariants());
  }

  Type(const Type&) noexcept;
  Type(Type&&) noexcept;
  Type& operator=(const Type&) noexcept;
  Type& operator=(Type&&) noexcept;
  ~Type() noexcept;

  /*
   * Exact equality or inequality of types.
   */
  bool operator==(const Type& o) const;
  bool operator!=(const Type& o) const { return !(*this == o); }

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
  friend bool is_specialized_array(const Type&);
  friend Type wait_handle_inner(const Type&);
  friend Type sval(SString);
  friend Type ival(int64_t);
  friend Type dval(double);
  friend Type aval(SArray);
  friend Type subObj(res::Class);
  friend Type objExact(res::Class);
  friend Type subCls(res::Class);
  friend Type clsExact(res::Class);
  friend Type arr_packed(std::vector<Type>);
  friend Type sarr_packed(std::vector<Type>);
  friend Type carr_packed(std::vector<Type>);
  friend Type arr_packedn(Type);
  friend Type sarr_packedn(Type);
  friend Type carr_packedn(Type);
  friend Type arr_struct(StructMap);
  friend Type sarr_struct(StructMap);
  friend Type carr_struct(StructMap);
  friend Type arr_mapn(Type k, Type v);
  friend Type sarr_mapn(Type k, Type v);
  friend Type carr_mapn(Type k, Type v);
  friend DObj dobj_of(const Type&);
  friend DCls dcls_of(Type);
  friend Type union_of(Type, Type);
  friend Type widening_union(const Type&, const Type&);
  friend Type opt(Type);
  friend Type unopt(Type);
  friend bool is_opt(Type);
  friend folly::Optional<Cell> tv(Type);
  friend std::string show(Type);
  friend struct ArrKey disect_key(const Type&);
  friend Type array_elem(const Type&, const Type&);
  friend Type array_set(Type, const Type&, const Type&);
  friend std::pair<Type,Type> array_newelem_key(const Type&, const Type&);
  friend std::pair<Type,Type> iter_types(const Type&);

private:
  union Data {
    Data() {}
    ~Data() {}

    SString sval;
    int64_t ival;
    double dval;
    SArray aval;
    DObj dobj;
    DCls dcls;
    copy_ptr<DArrPacked> apacked;
    copy_ptr<DArrPackedN> apackedn;
    copy_ptr<DArrStruct> astruct;
    copy_ptr<DArrMapN> amapn;
  };

  template<class Ret, class T, class Function>
  struct DJHelperFn;
  struct ArrKey;

private:
  static Type wait_handle_outer(const Type&);
  static Type unionArr(const Type& a, const Type& b);

private:
  template<class Ret, class T, class Function>
  Ret dj2nd(const Type&, DJHelperFn<Ret,T,Function>) const;
  template<class Function>
  typename Function::result_type disjointDataFn(const Type&, Function) const;
  bool hasData() const;
  bool equivData(const Type&) const;
  bool subtypeData(const Type&) const;
  bool couldBeData(const Type&) const;
  bool checkInvariants() const;

private:
  trep m_bits;
  DataTag m_dataTag = DataTag::None;
  Data m_data;
};

//////////////////////////////////////////////////////////////////////

struct DArrPacked {
  explicit DArrPacked(std::vector<Type> elems)
    : elems(std::move(elems))
  {}

  std::vector<Type> elems;
};

struct DArrPackedN {
  explicit DArrPackedN(Type t) : type(std::move(t)) {}
  Type type;
};

struct DArrStruct {
  explicit DArrStruct(StructMap map) : map(std::move(map)) {}
  StructMap map;
};

struct DArrMapN {
  explicit DArrMapN(Type key, Type val)
    : key(std::move(key))
    , val(std::move(val))
  {}
  Type key;
  Type val;
};

//////////////////////////////////////////////////////////////////////

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
X(SArrE)
X(CArrE)
X(SArrN)
X(CArrN)
X(Obj)
X(Res)
X(Cls)
X(Ref)

X(Null)
X(Bool)
X(Num)
X(Str)
X(SArr)
X(CArr)
X(ArrE)
X(ArrN)
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
X(OptSArrE)
X(OptCArrE)
X(OptSArrN)
X(OptCArrN)
X(OptSArr)
X(OptCArr)
X(OptArrE)
X(OptArrN)
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
 * Create static empty array or string types.
 */
Type sempty();
Type aempty();

/*
 * Create a reference counted empty array.
 */
Type counted_aempty();

/*
 * Create types for objects or classes with some known constraint on
 * which res::Class is associated with them.
 */
Type subObj(res::Class);
Type objExact(res::Class);
Type subCls(res::Class);
Type clsExact(res::Class);

/*
 * Packed array types with known size.
 *
 * Pre: !v.empty()
 */
Type arr_packed(std::vector<Type> v);
Type sarr_packed(std::vector<Type> v);
Type carr_packed(std::vector<Type> v);

/*
 * Packed array types of unknown size.
 *
 * Note that these types imply the arrays are non-empty.
 */
Type arr_packedn(Type);
Type sarr_packedn(Type);
Type carr_packedn(Type);

/*
 * Struct-like arrays.
 *
 * Pre: !m.empty()
 */
Type arr_struct(StructMap m);
Type sarr_struct(StructMap m);
Type carr_struct(StructMap m);

/*
 * Map-like arrays.
 */
Type arr_mapn(Type k, Type v);
Type sarr_mapn(Type k, Type v);
Type carr_mapn(Type k, Type v);

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
 * Returns whether `t' is a Arr=something or Arr(something) type, or
 * an optional version of one of those types.  That is, an array with
 * either a constant value or some (maybe partially) known shape.
 */
bool is_specialized_array(const Type& t);

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
 * accurate.  `s' may be nullptr.
 */
Type from_hni_constraint(SString s);

/*
 * Make a type that represents values from either of the supplied
 * types.
 *
 * Importantly, note that there are infinitely long chains of array
 * types that continue to become less specialized, so chains of
 * union_of operations are not guaranteed to reach a stable point in
 * finite steps.
 */
Type union_of(Type a, Type b);

/*
 * Widening union.
 *
 * This operation returns a type T, such that a is a subtype of T, b
 * is a subtype of T, and union_of(a, b) is a subtype of T.  The
 * widening union also has the property that every possible chain of
 * successive applications of the function eventually reaches a stable
 * point.
 *
 * For portions of our analysis that rely on growing types reaching
 * stable points for termination, this function must occasionally be
 * used instead of union_of to guarantee termination.  See details in
 * analyze.cpp.
 */
Type widening_union(const Type& a, const Type& b);

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
 *
 * TODO(#3696042): loosen values of an array shape should keep the
 * shape.
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

/*
 * Returns the best known type of an array inner element given a type
 * for the key.  The returned type is always a subtype of TInitCell.
 *
 * Pre: arr.subtypeOf(TArr)
 */
Type array_elem(const Type& arr, const Type& key);

/*
 * Perform an array set on types.  Returns a type that represents the
 * effects of arr[key] = val.
 *
 * Pre arr.subtypeOf(TArr)
 */
Type array_set(Type arr, const Type& key, const Type& val);

/*
 * Perform a newelem operation on an array type.  Returns an array
 * that contains a new pushed-back element with the supplied value, in
 * the sense of arr[] = val.
 *
 * Pre: arr.subtypeOf(TArr)
 */
Type array_newelem(const Type& arr, const Type& val);

/*
 * The same as array_newelem, except return the best known type of the
 * key that was added.  (This is either TInt or a subtype of it.)
 */
std::pair<Type,Type> array_newelem_key(const Type& arr, const Type& val);

/*
 * Return the best known key and value type for iteration of the
 * supplied type.  This is only intended for non-mutable iteration, so
 * the returned types are at worst InitCell.
 */
std::pair<Type,Type> iter_types(const Type&);

//////////////////////////////////////////////////////////////////////

}}

#endif
