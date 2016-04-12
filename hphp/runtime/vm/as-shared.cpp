/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/as-shared.h"

#include <folly/gen/Base.h>
#include <folly/gen/String.h>

#include <vector>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

using ContextMask = uint32_t;

constexpr auto C = static_cast<ContextMask>(AttrContext::Class);
constexpr auto F = static_cast<ContextMask>(AttrContext::Func);
constexpr auto P = static_cast<ContextMask>(AttrContext::Prop);
constexpr auto T = static_cast<ContextMask>(AttrContext::TraitImport);
constexpr auto A = static_cast<ContextMask>(AttrContext::Alias);

constexpr bool supported(ContextMask mask, AttrContext a) {
  return mask & static_cast<ContextMask>(a);
}

#define HHAS_ATTRS                                          \
  X(AttrPublic,               F|P|T,   "public");           \
  X(AttrProtected,            F|P|T,   "protected");        \
  X(AttrPrivate,              F|P|T,   "private");          \
  X(AttrStatic,               F|P,     "static");           \
  X(AttrEnum,                 C,       "enum");             \
  X(AttrDeepInit,             P,       "deep_init");        \
  X(AttrInterface,            C,       "interface");        \
  X(AttrNoExpandTrait,        C,       "no_expand_trait");  \
  X(AttrAbstract,             C|F|T,   "abstract");         \
  X(AttrNoOverride,           C|F|T,   "no_override");      \
  X(AttrFinal,                C|F|T,   "final");            \
  X(AttrTrait,                C|F,     "trait");            \
  X(AttrUnique,               C|F,     "unique");           \
  X(AttrBuiltin,              C|F,     "builtin");          \
  X(AttrPersistent,           C|F|A,   "persistent");       \
  X(AttrNoOverrideMagicGet,   C,       "nov_get");          \
  X(AttrNoOverrideMagicSet,   C,       "nov_set");          \
  X(AttrNoOverrideMagicIsset, C,       "nov_isset");        \
  X(AttrNoOverrideMagicUnset, C,       "nov_unset");        \
  X(AttrMayUseVV,             F,       "mayusevv");         \
  /* */

#define HHAS_TYPE_FLAGS                                     \
  X(Nullable,        "nullable");                           \
  X(HHType,          "hh_type");                            \
  X(ExtendedHint,    "extended_hint");                      \
  X(TypeVar,         "type_var");                           \
  X(Soft,            "soft");                               \
  X(TypeConstant,    "type_constant")

}

//////////////////////////////////////////////////////////////////////

std::string attrs_to_string(AttrContext ctx, Attr attrs) {
  std::vector<std::string> vec;

#define X(attr, mask, str) \
  if (supported(mask, ctx) && (attrs & attr)) vec.push_back(str);
  HHAS_ATTRS
#undef X

  using namespace folly::gen;
  return from(vec) | unsplit<std::string>(" ");
}

folly::Optional<Attr> string_to_attr(AttrContext ctx,
                                     const std::string& name) {
#define X(attr, mask, str) \
  if (supported(mask, ctx) && name == str) return attr;
  HHAS_ATTRS
#undef X

return folly::none;
}

//////////////////////////////////////////////////////////////////////

std::string type_flags_to_string(TypeConstraint::Flags flags) {
  std::vector<std::string> vec;

#define X(flag, str) \
  if (flags & TypeConstraint::flag) vec.push_back(str);
  HHAS_TYPE_FLAGS
#undef X

  using namespace folly::gen;
  return from(vec) | unsplit<std::string>(" ");
}

folly::Optional<TypeConstraint::Flags> string_to_type_flag(
  const std::string& name) {
#define X(flag, str) \
  if (name == str) return TypeConstraint::flag;
  HHAS_TYPE_FLAGS
#undef X

return folly::none;
}

//////////////////////////////////////////////////////////////////////

}
