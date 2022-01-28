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
namespace HPHP {
//////////////////////////////////////////////////////////////////////

/*
 * This header is needed to avoid combining source files as-attr-ffi.cpp
 * and as-shared.cpp to minimize the amount of dependencies in the ffi
 * between HHVM and HackC.
 */

//////////////////////////////////////////////////////////////////////
namespace {

using ContextMask = uint32_t;

constexpr auto C = static_cast<ContextMask>(AttrContext::Class);
constexpr auto F = static_cast<ContextMask>(AttrContext::Func);
constexpr auto P = static_cast<ContextMask>(AttrContext::Prop);
constexpr auto T = static_cast<ContextMask>(AttrContext::TraitImport);
constexpr auto A = static_cast<ContextMask>(AttrContext::Alias);
constexpr auto K = static_cast<ContextMask>(AttrContext::Constant);

constexpr bool supported(ContextMask mask, AttrContext a) {
  return mask & static_cast<ContextMask>(a);
}

#define HHAS_ATTRS                                                  \
  X(AttrPublic,                   F|P|T,   "public");               \
  X(AttrProtected,                F|P|T,   "protected");            \
  X(AttrPrivate,                  F|P|T,   "private");              \
  X(AttrStatic,                   F|P,     "static");               \
  X(AttrEnum,                     C,       "enum");                 \
  X(AttrDeepInit,                 P,       "deep_init");            \
  X(AttrInterface,                C,       "interface");            \
  X(AttrNoExpandTrait,            C,       "no_expand_trait");      \
  X(AttrAbstract,                 C|F|T,   "abstract");             \
  X(AttrNoOverride,               C|F|T,   "no_override");          \
  X(AttrFinal,                    C|F|T,   "final");                \
  X(AttrSealed,                   C,       "sealed");               \
  X(AttrTrait,                    C|F|P,   "trait");                \
  X(AttrUnique,                   C|F,     "unique");               \
  X(AttrBuiltin,                  C|F,     "builtin");              \
  X(AttrPersistent,               C|F|A|K, "persistent");           \
  X(AttrIsConst,                  C|P,     "is_const");             \
  X(AttrIsReadonly,               P,       "readonly");             \
  X(AttrReadonlyReturn,           F,       "readonly_return");      \
  X(AttrReadonlyThis,             F,       "readonly_this");        \
  X(AttrForbidDynamicProps,       C,       "no_dynamic_props");     \
  X(AttrDynamicallyConstructible, C,       "dyn_constructible");    \
  X(AttrProvenanceSkipFrame,      F,       "prov_skip_frame");      \
  X(AttrIsFoldable,               F,       "foldable");             \
  X(AttrNoInjection,              F,       "no_injection");         \
  X(AttrInterceptable,            F,       "interceptable");        \
  X(AttrDynamicallyCallable,      F,       "dyn_callable");         \
  X(AttrLSB,                      P,       "lsb");                  \
  X(AttrNoBadRedeclare,           P,       "no_bad_redeclare");     \
  X(AttrSystemInitialValue,       P,       "sys_initial_val");      \
  X(AttrNoImplicitNullable,       P,       "no_implicit_null");     \
  X(AttrInitialSatisfiesTC,       P,       "initial_satisfies_tc"); \
  X(AttrLateInit,                 P,       "late_init");            \
  X(AttrNoReifiedInit,            C,       "noreifiedinit");        \
  X(AttrIsMethCaller,             F,       "is_meth_caller");       \
  X(AttrEnumClass,                C,       "enum_class");
  /* */
}

}
