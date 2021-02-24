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

#ifndef HHBBC_TYPE_SYSTEM_DETAIL_H
#error "type-system-detail.h should only be included by type-system-bits.h"
#endif

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/cat.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/pop_back.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/variadic/to_tuple.hpp>

/*
 * This file is responsible for taking the various type declaration
 * macros declared in type-system-bits.h and using them to create all
 * the appropriate trep bits. It's split into a separate file because
 * the details are ugly.
 *
 * Besides the treps themselves, it also declares the
 * HHBBC_TYPE_PREDEFINED macro which can be used to expand to all
 * predefined types.
 */

//////////////////////////////////////////////////////////////////////

// These are all internal macros. They cannot be undefined afterward
// this because they're used in the expansion of
// HHBBC_TYPE_PREDEFINED.

#define HHBBC_COUNTED_OR_UNCOUNTEDSTATIC_ELEMS(Name, X)  \
  X(S##Name)                                             \

#define HHBBC_COUNTED_OR_UNCOUNTEDCOUNTED_ELEMS(Name, X) \
  X(C##Name)                                             \

#define HHBBC_COUNTED_OR_UNCOUNTEDELEMS(Name, X)     \
  HHBBC_COUNTED_OR_UNCOUNTEDSTATIC_ELEMS(Name, X)    \
  HHBBC_COUNTED_OR_UNCOUNTEDCOUNTED_ELEMS(Name, X)   \

#define HHBBC_COUNTED_OR_UNCOUNTEDUNIONS(Name, X)    \
  X(Name, BS##Name|BC##Name)                         \

#define HHBBC_ARRAY_STATIC_ELEMS(Name, X, ...)  \
  X(S##Name##E)                                 \
  X(S##Name##N)                                 \

#define HHBBC_ARRAY_COUNTED_ELEMS(Name, X, ...) \
  X(C##Name##E)                                 \
  X(C##Name##N)                                 \

#define HHBBC_ARRAY_ELEMS(Name, X)              \
  HHBBC_ARRAY_STATIC_ELEMS(Name, X)             \
  HHBBC_ARRAY_COUNTED_ELEMS(Name, X)            \

#define HHBBC_ARRAY_UNIONS(Name, X, ...)        \
  X(S##Name, BS##Name##E|BS##Name##N)           \
  X(C##Name, BC##Name##E|BC##Name##N)           \
  X(Name##E, BS##Name##E|BC##Name##E)           \
  X(Name##N, BS##Name##N|BC##Name##N)           \
  X(Name,    B##Name##E|B##Name##N)             \

#define HHBBC_ADD_PREFIX_SUFFIX(_, Data, Elem)                          \
  BOOST_PP_CAT(                                                         \
    BOOST_PP_CAT(                                                       \
      BOOST_PP_TUPLE_ELEM(0, Data),Elem),BOOST_PP_TUPLE_ELEM(1, Data))  \

#define HHBBC_JOIN_WITH_UNION(_, Prev, X)  Prev|X

#define BUILD_UNION_EXPR(Seq)                                          \
   BOOST_PP_SEQ_FOLD_LEFT(                                             \
     HHBBC_JOIN_WITH_UNION,                                            \
     BOOST_PP_SEQ_HEAD(Seq), BOOST_PP_SEQ_TAIL(Seq))

#define HHBBC_TRANSFORM_ARRAY_LIST(Prefix, Suffix, ...)                 \
  BUILD_UNION_EXPR(BOOST_PP_SEQ_TRANSFORM(                              \
          HHBBC_ADD_PREFIX_SUFFIX,                                      \
          BOOST_PP_VARIADIC_TO_TUPLE(Prefix, Suffix),                   \
          BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))                       \

#define HHBBC_ARRAY_UNION_ELEMS(Name, X, ...)                        \
  X(S##Name##E, HHBBC_TRANSFORM_ARRAY_LIST(BS, E, __VA_ARGS__))      \
  X(C##Name##E, HHBBC_TRANSFORM_ARRAY_LIST(BC, E, __VA_ARGS__))      \
  X(S##Name##N, HHBBC_TRANSFORM_ARRAY_LIST(BS, N, __VA_ARGS__))      \
  X(C##Name##N, HHBBC_TRANSFORM_ARRAY_LIST(BC, N, __VA_ARGS__))      \

#define HHBBC_NAME_TO_UNION_FRAGMENT(Name) B##Name |

#define HHBBC_UNCOUNTED_TYPES                                           \
  HHBBC_TYPE_DECL_SINGLE_UNCOUNTED_NO_OPT(HHBBC_NAME_TO_UNION_FRAGMENT) \
  HHBBC_TYPE_DECL_SINGLE_UNCOUNTED(HHBBC_NAME_TO_UNION_FRAGMENT)        \
  HHBBC_TYPE_DECL_COUNTED_OR_UNCOUNTED(                                 \
    HHBBC_COUNTED_OR_UNCOUNTEDSTATIC_ELEMS,                             \
    HHBBC_NAME_TO_UNION_FRAGMENT)                                       \
  HHBBC_TYPE_DECL_ARRAY(                                                \
    HHBBC_ARRAY_STATIC_ELEMS,                                           \
    HHBBC_NAME_TO_UNION_FRAGMENT)                                       \
  HHBBC_TYPE_DECL_ARRAY_UNION(                                          \
    HHBBC_ARRAY_STATIC_ELEMS,                                           \
    HHBBC_NAME_TO_UNION_FRAGMENT)                                       \
  0                                                                     \

#define HHBBC_COUNTED_TYPES                                             \
  HHBBC_TYPE_DECL_SINGLE_COUNTED_NO_OPT(HHBBC_NAME_TO_UNION_FRAGMENT)   \
  HHBBC_TYPE_DECL_SINGLE_COUNTED(HHBBC_NAME_TO_UNION_FRAGMENT)          \
  HHBBC_TYPE_DECL_COUNTED_OR_UNCOUNTED(                                 \
    HHBBC_COUNTED_OR_UNCOUNTEDCOUNTED_ELEMS,                            \
    HHBBC_NAME_TO_UNION_FRAGMENT)                                       \
  HHBBC_TYPE_DECL_ARRAY(                                                \
    HHBBC_ARRAY_COUNTED_ELEMS,                                          \
    HHBBC_NAME_TO_UNION_FRAGMENT)                                       \
  HHBBC_TYPE_DECL_ARRAY_UNION(                                          \
    HHBBC_ARRAY_COUNTED_ELEMS,                                          \
    HHBBC_NAME_TO_UNION_FRAGMENT)                                       \
  0                                                                     \

#define HHBBC_TYPE_SINGLE_OPT(X)                                           \
  HHBBC_TYPE_DECL_SINGLE_UNCOUNTED(X)                                      \
  HHBBC_TYPE_DECL_SINGLE_COUNTED(X)                                        \
  HHBBC_TYPE_DECL_COUNTED_OR_UNCOUNTED(HHBBC_COUNTED_OR_UNCOUNTEDELEMS, X) \
  HHBBC_TYPE_DECL_ARRAY(HHBBC_ARRAY_ELEMS, X)                              \

#define HHBBC_TYPE_SINGLE(X)                          \
  HHBBC_TYPE_DECL_SINGLE_UNCOUNTED_NO_OPT(X)          \
  HHBBC_TYPE_DECL_SINGLE_COUNTED_NO_OPT(X)            \
  HHBBC_TYPE_SINGLE_OPT(X)                            \

#define HHBBC_TYPE_UNION_OPT(X)                       \
  HHBBC_TYPE_DECL_COUNTED_OR_UNCOUNTED(HHBBC_COUNTED_OR_UNCOUNTEDUNIONS, X) \
  HHBBC_TYPE_DECL_ARRAY(HHBBC_ARRAY_UNIONS, X)                              \
  HHBBC_TYPE_DECL_ARRAY_UNION(HHBBC_ARRAY_UNION_ELEMS, X)                   \
  HHBBC_TYPE_DECL_ARRAY_UNION(HHBBC_ARRAY_UNIONS, X)                        \
  HHBBC_TYPE_DECL_UNION(X)                                                  \

#define HHBBC_CELL_TYPES                              \
  HHBBC_TYPE_SINGLE(HHBBC_NAME_TO_UNION_FRAGMENT)     \
  0                                                   \

#define HHBBC_TYPE_UNION(X)                           \
  HHBBC_TYPE_UNION_OPT(X)                             \
  HHBBC_TYPE_DECL_UNION_NO_OPT(X)                     \
  X(Unc, HHBBC_UNCOUNTED_TYPES)                       \
  X(InitUnc, BUnc - BUninit)                          \
  X(Counted, HHBBC_COUNTED_TYPES)                     \
  X(Cell, HHBBC_CELL_TYPES)                           \
  X(NonNull, BCell - BNull)                           \
  X(InitCell, BCell - BUninit)                        \

#define HHBBC_MAKE_NAME_OPT(Name, ...) (Opt##Name)
#define HHBBC_CALL_X_WITH_NAME(_, X, Name) X(Name)

//////////////////////////////////////////////////////////////////////

// These is meant to be used outside this file to expand to all
// predefined types:

#define HHBBC_TYPE_PREDEFINED(X)                      \
  X(Bottom)                                           \
  HHBBC_TYPE_SINGLE(X)                                \
  HHBBC_TYPE_UNION(X)                                 \
  BOOST_PP_SEQ_FOR_EACH(                              \
    HHBBC_CALL_X_WITH_NAME,                           \
    X,                                                \
    HHBBC_TYPE_SINGLE_OPT(HHBBC_MAKE_NAME_OPT))       \
  BOOST_PP_SEQ_FOR_EACH(                              \
    HHBBC_CALL_X_WITH_NAME,                           \
    X,                                                \
    HHBBC_TYPE_UNION_OPT(HHBBC_MAKE_NAME_OPT))        \
  X(Top)                                              \

//////////////////////////////////////////////////////////////////////

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace detail {

// This is just an enumeration of all non-union types. It's used to
// automatically generate a bit index for each such type in the trep.
enum class single_types {
  #define X(Name) Name,
  HHBBC_TYPE_SINGLE(X)
  #undef X
  Max
};

}

//////////////////////////////////////////////////////////////////////

constexpr const size_t kTRepBitsStored = 48;

static_assert((size_t)detail::single_types::Max <= kTRepBitsStored);

enum trep : uint64_t {
  BBottom = 0,

  // First the non-union types. They each get a specific bit index.
  #define X(Name) B##Name = 1ULL << (int64_t)detail::single_types::Name,
  HHBBC_TYPE_SINGLE(X)
  #undef X

  // Then the union types. These are just combinations of the above
  // non-union types.
  #define X(Name, Bits) B##Name = (Bits),
  HHBBC_TYPE_UNION(X)
  #undef X

  // The non-union types which can be made optional. This is just the
  // type with BInitNull added.
  #define X(Name) BOpt##Name = B##Name | BInitNull,
  HHBBC_TYPE_SINGLE_OPT(X)
  #undef X

  // Finally the same, except for union types.
  #define X(Name, ...) BOpt##Name = B##Name | BInitNull,
  HHBBC_TYPE_UNION_OPT(X)
  #undef X

  BTop      = (1ULL << kTRepBitsStored) - 1,
};

//////////////////////////////////////////////////////////////////////

}}
