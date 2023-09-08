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

//////////////////////////////////////////////////////////////////////

#define REPO_AUTH_TYPE_TAGS_NO_OPT(X) \
  X(Uninit, "Uninit")                 \
  X(InitNull, "InitNull")             \
  X(Null, "Null")                     \
  X(InitPrim, "InitPrim")             \
  X(InitUnc, "InitUnc")               \
  X(Unc, "Unc")                       \
  X(NonNull, "NonNull")               \
  X(InitCell, "InitCell")             \
  X(Cell, "Cell")                     \

#define REPO_AUTH_TYPE_TAGS_CAN_BE_OPT(X, Y) \
  X(Dbl, Y)                                  \
  X(Res, Y)                                  \
  X(Func, Y)                                 \
  X(ClsMeth, Y)                              \
  X(LazyCls, Y)                              \
  X(EnumClassLabel, Y)                       \
  X(Num, Y)                                  \
  X(UncArrKey, Y)                            \
  X(ArrKey, Y)                               \
  X(UncStrLike, Y)                           \
  X(StrLike, Y)                              \
  X(UncArrKeyCompat, Y)                      \
  X(ArrKeyCompat, Y)                         \
  X(VecCompat, Y)                            \
  X(ArrLike, Y)                              \
  X(SArrLike, Y)                             \
  X(ArrLikeCompat, Y)                        \

#define REPO_AUTH_TYPE_TAGS_CAN_BE_OPT_AND_UNINIT(X, Y) \
  X(Int, Y)                                             \
  X(Bool, Y)                                            \
  X(Str, Y)                                             \
  X(SStr, Y)                                            \

#define REPO_AUTH_TYPE_TAGS_CAN_HAVE_NAME(X, Y) \
  X(Cls, Y)                                     \

#define REPO_AUTH_TYPE_TAGS_CAN_HAVE_NAME_AND_UNINIT(X, Y) \
  X(Obj, Y)                                                \

#define REPO_AUTH_TYPE_TAGS_ARRAY(X, Y) \
  X(Vec, Y)                             \
  X(Dict, Y)                            \
  X(Keyset, Y)                          \

//////////////////////////////////////////////////////////////////////

#define REPO_AUTH_TYPE_TAGS_DETAIL_ADD_OPT(Tag, Y) \
  Y(Tag, #Tag)                                     \
  Y(Opt##Tag, "?"#Tag)                             \

#define REPO_AUTH_TYPE_TAGS_DETAIL_ADD_UNINIT(Tag, Y) \
  Y(Tag, #Tag)                                        \
  Y(Opt##Tag, "?"#Tag)                                \
  Y(Uninit##Tag, "Uninit"#Tag)                        \

#define REPO_AUTH_TYPE_TAGS_DETAIL_ADD_STATIC(Tag, Y) \
  Y(Tag, #Tag)                                        \
  Y(S##Tag, "S"#Tag)                                  \
  Y(Opt##Tag, "?"#Tag)                                \
  Y(OptS##Tag, "?S"#Tag)                              \

#define REPO_AUTH_TYPE_TAGS_DETAIL_ADD_SUB_NAME(Tag, Y) \
  Y(Sub##Tag, #Tag"<=")                                 \
  Y(OptSub##Tag, "?"#Tag"<=")                           \

#define REPO_AUTH_TYPE_TAGS_DETAIL_ADD_EXACT_NAME(Tag, Y) \
  Y(Exact##Tag, #Tag"=")                                  \
  Y(OptExact##Tag, "?"#Tag"=")                            \

#define REPO_AUTH_TYPE_TAGS_DETAIL_ADD_SUB_NAME_AND_UNINIT(Tag, Y) \
  REPO_AUTH_TYPE_TAGS_DETAIL_ADD_SUB_NAME(Tag, Y)                  \
  Y(UninitSub##Tag, "Uninit"#Tag"<=")                              \

#define REPO_AUTH_TYPE_TAGS_DETAIL_ADD_EXACT_NAME_AND_UNINIT(Tag, Y) \
  REPO_AUTH_TYPE_TAGS_DETAIL_ADD_EXACT_NAME(Tag, Y)                  \
  Y(UninitExact##Tag, "Uninit"#Tag"=")                               \

#define REPO_AUTH_TYPE_TAGS_DETAIL_ADD_NAME(Tag, Y) \
  REPO_AUTH_TYPE_TAGS_DETAIL_ADD_SUB_NAME(Tag, Y)   \
  REPO_AUTH_TYPE_TAGS_DETAIL_ADD_EXACT_NAME(Tag, Y) \

#define REPO_AUTH_TYPE_TAGS_DETAIL_ADD_NAME_AND_UNINIT(Tag, Y) \
  REPO_AUTH_TYPE_TAGS_DETAIL_ADD_SUB_NAME_AND_UNINIT(Tag, Y)   \
  REPO_AUTH_TYPE_TAGS_DETAIL_ADD_EXACT_NAME_AND_UNINIT(Tag, Y) \

#define REPO_AUTH_TYPE_TAGS_DETAIL_ADD_SPEC(Tag, Y) \
  Y(Tag##Spec, #Tag)                                \
  Y(S##Tag##Spec, "S"#Tag)                          \
  Y(Opt##Tag##Spec, "?"#Tag)                        \
  Y(OptS##Tag##Spec, "?S"#Tag)                      \

//////////////////////////////////////////////////////////////////////

#define REPO_AUTH_TYPE_TAGS_HAS_ARRSPEC(X)                            \
  REPO_AUTH_TYPE_TAGS_ARRAY(REPO_AUTH_TYPE_TAGS_DETAIL_ADD_SPEC, X)   \

#define REPO_AUTH_TYPE_TAGS_HAS_NAME(X)                                     \
  REPO_AUTH_TYPE_TAGS_CAN_HAVE_NAME(REPO_AUTH_TYPE_TAGS_DETAIL_ADD_NAME, X) \
  REPO_AUTH_TYPE_TAGS_CAN_HAVE_NAME_AND_UNINIT(                             \
    REPO_AUTH_TYPE_TAGS_DETAIL_ADD_NAME_AND_UNINIT, X                       \
  )                                                                         \

#define REPO_AUTH_TYPE_TAGS_NO_DATA(X)                                     \
  REPO_AUTH_TYPE_TAGS_NO_OPT(X)                                            \
  REPO_AUTH_TYPE_TAGS_CAN_BE_OPT(REPO_AUTH_TYPE_TAGS_DETAIL_ADD_OPT, X)    \
  REPO_AUTH_TYPE_TAGS_CAN_BE_OPT_AND_UNINIT(                               \
    REPO_AUTH_TYPE_TAGS_DETAIL_ADD_UNINIT, X                               \
  )                                                                        \
  REPO_AUTH_TYPE_TAGS_CAN_HAVE_NAME(REPO_AUTH_TYPE_TAGS_DETAIL_ADD_OPT, X) \
  REPO_AUTH_TYPE_TAGS_CAN_HAVE_NAME_AND_UNINIT(                            \
    REPO_AUTH_TYPE_TAGS_DETAIL_ADD_UNINIT, X                               \
  )                                                                        \
  REPO_AUTH_TYPE_TAGS_ARRAY(REPO_AUTH_TYPE_TAGS_DETAIL_ADD_STATIC, X)      \

#define REPO_AUTH_TYPE_TAGS_SUB_NAME(X)                     \
  REPO_AUTH_TYPE_TAGS_CAN_HAVE_NAME(                        \
    REPO_AUTH_TYPE_TAGS_DETAIL_ADD_SUB_NAME, X              \
  )                                                         \
  REPO_AUTH_TYPE_TAGS_CAN_HAVE_NAME_AND_UNINIT(             \
    REPO_AUTH_TYPE_TAGS_DETAIL_ADD_SUB_NAME_AND_UNINIT, X   \
  )                                                         \

#define REPO_AUTH_TYPE_TAGS_EXACT_NAME(X)                   \
  REPO_AUTH_TYPE_TAGS_CAN_HAVE_NAME(                        \
    REPO_AUTH_TYPE_TAGS_DETAIL_ADD_EXACT_NAME, X            \
  )                                                         \
  REPO_AUTH_TYPE_TAGS_CAN_HAVE_NAME_AND_UNINIT(             \
    REPO_AUTH_TYPE_TAGS_DETAIL_ADD_EXACT_NAME_AND_UNINIT, X \
  )                                                         \

#define REPO_AUTH_TYPE_TAGS_DATA(X)  \
  REPO_AUTH_TYPE_TAGS_HAS_NAME(X)    \
  REPO_AUTH_TYPE_TAGS_HAS_ARRSPEC(X) \

#define REPO_AUTH_TYPE_TAGS(X)   \
  REPO_AUTH_TYPE_TAGS_NO_DATA(X) \
  REPO_AUTH_TYPE_TAGS_DATA(X)    \

//////////////////////////////////////////////////////////////////////
