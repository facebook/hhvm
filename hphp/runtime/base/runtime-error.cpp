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

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/logger.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/string-vsnprintf.h"
#include "hphp/util/struct-log.h"

#include <folly/AtomicHashMap.h>
#include <folly/logging/RateLimiter.h>
#include <folly/Range.h>

#ifdef ERROR
# undef ERROR
#endif
#ifdef STRICT
# undef STRICT
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Careful in these functions: they can be called when tl_regState is
 * DIRTY.  ExecutionContext::handleError is dirty-reg safe, but
 * evaluate other functions that you might need here.
 */

void raise_error(const std::string &msg) {
  g_context->handleError(msg, static_cast<int>(ErrorMode::ERROR), false,
                         ExecutionContext::ErrorThrowMode::Always,
                         "\nFatal error: ",
                         false);
  always_assert(0);
}

void raise_error(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_error(msg);
}

void raise_error_without_first_frame(const std::string &msg) {
  g_context->handleError(msg, static_cast<int>(ErrorMode::ERROR), false,
                         ExecutionContext::ErrorThrowMode::Always,
                         "\nFatal error: ",
                         true);
  always_assert(0);
}

void raise_recoverable_error(const std::string &msg) {
  g_context->handleError(
    msg, static_cast<int>(ErrorMode::RECOVERABLE_ERROR), true,
    ExecutionContext::ErrorThrowMode::IfUnhandled,
    "\nCatchable Fatal error: ",
    false);
}

void raise_recoverable_error_without_first_frame(const std::string &msg) {
  g_context->handleError(
    msg, static_cast<int>(ErrorMode::RECOVERABLE_ERROR), true,
    ExecutionContext::ErrorThrowMode::IfUnhandled,
    "\nCatchable Fatal error: ",
    true);
}

void raise_typehint_error(const std::string& msg) {
  if (RuntimeOption::PHP7_EngineExceptions) {
    VMRegAnchor _;
    SystemLib::throwTypeErrorObject(msg);
  }
  raise_recoverable_error_without_first_frame(msg);
  raise_error("Error handler tried to recover from typehint violation");
}

void raise_reified_typehint_error(const std::string& msg, bool warn) {
  if (!warn) return raise_typehint_error(msg);
  raise_warning_unsampled(msg);
}

void raise_return_typehint_error(const std::string& msg) {
  if (RuntimeOption::PHP7_EngineExceptions) {
    VMRegAnchor _;
    SystemLib::throwTypeErrorObject(msg);
  }
  raise_recoverable_error(msg);
  raise_error("Error handler tried to recover from a return typehint "
              "violation");
}

void raise_property_typehint_error(const std::string& msg,
                                   bool isSoft, bool isUB) {
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);

  if (RuntimeOption::EvalCheckPropTypeHints == 1 ||
      isSoft ||
      (isUB && RuntimeOption::EvalEnforceGenericsUB < 2)) {
    raise_warning_unsampled(msg);
    return;
  }

  raise_recoverable_error(msg);
  if (RuntimeOption::EvalCheckPropTypeHints >= 3) {
    raise_error("Error handler tried to recover from a property typehint "
                "violation");
  }
}

void raise_record_field_typehint_error(const std::string& msg, bool isSoft) {
  if (isSoft) {
    raise_warning_unsampled(msg);
    return;
  }
  raise_recoverable_error(msg);
  raise_error("Error handler tried to recover from a record field typehint "
              "violation");
}

void raise_record_init_error(const StringData* recName,
                             const StringData* fieldName) {
  raise_error(
    folly::sformat("Record field '{}::{}' not initialized", recName, fieldName)
  );
}

void raise_record_field_error(const StringData* recName,
                              const StringData* fieldName) {
  raise_error(folly::sformat("Field '{}' does not exist in record '{}'",
                              fieldName, recName));
}

void raise_property_typehint_unset_error(const Class* declCls,
                                         const StringData* propName,
                                         bool isSoft, bool isUB) {
  raise_property_typehint_error(
    folly::sformat(
      "Unsetting property '{}::{}' with type annotation",
      declCls->name(),
      propName
    ),
    isSoft,
    isUB
  );
}

void raise_convert_object_to_string(const char* cls_name) {
  raise_error("Cannot convert object to string (got instance of %s)", cls_name);
}

void raise_convert_record_to_type(const char* typeName) {
  raise_error("Cannot convert record to %s", typeName);
}

void raise_convert_rfunc_to_type(const char* typeName) {
  raise_error("Cannot convert reified function to %s", typeName);
}

void raise_convert_rcls_meth_to_type(const char* typeName) {
  raise_error("Cannot convert reified class meth pointer to %s", typeName);
}

void raise_use_of_specialized_array() {
  raise_error(Strings::DATATYPE_SPECIALIZED_DVARR);
}

void raise_hackarr_compat_notice(const std::string& msg) {
  raise_notice("Hack Array Compat: %s", msg.c_str());
}

#define HC(Opt, opt) \
  void raise_hac_##opt##_notice(const std::string& msg) {       \
    if (UNLIKELY(RID().getSuppressHAC##Opt##Notices())) return; \
    raise_notice("Hack Array Compat: %s", msg.c_str());         \
  }
HAC_CHECK_OPTS
#undef HC

void raise_hack_arr_compat_serialize_notice(const ArrayData* arr) {
  auto const type = [&]{
    if (arr->isVecType()) return "vec";
    if (arr->isDictType())     return "dict";
    if (arr->isKeysetType())   return "keyset";
    return "array";
  }();
  raise_notice("Hack Array Compat: Serializing %s", type);
}

namespace {

folly::Synchronized<
  folly::F14FastSet<std::pair<PC, std::string>>,
  std::mutex
  > g_previouslyRaisedNotices;

template <typename... Args>
void raise_dynamically_sampled_notice(folly::StringPiece fmt, Args&& ... args) {
  static auto samplingTableSize = ServiceData::createTimeSeries(
    "vm.dynsampling.table-size",
    {ServiceData::StatsType::SUM}
  );

  /*
   * We want to dedupe notices, but not so much that we exclude
   * notices at a new location, so we need to grab the first
   * not-skipframe saved PC, since this is the first frame that will
   * be listed in the backtrace
   */
  VMRegAnchor _;
  auto const pc = [&] {
    if (LIKELY(!vmfp()->skipFrame())) return vmpc();
    Offset offset;
    auto ar = g_context->getPrevVMStateSkipFrame(vmfp(), &offset);
    return ar->func()->at(offset);
  }();

  auto const str = folly::sformat(fmt, std::move(args) ...);
  {
    auto notices = g_previouslyRaisedNotices.lock();
    auto const inserted = notices->emplace(pc, str);
    if (!inserted.second) return;
  }
  samplingTableSize->addValue(1);
  raise_notice(str);
}

enum class ArrayType {
  VArray,
  DArray,
  Vec,
  Dict,
  Other,
  Count
};

const char* arrayTypeName(ArrayType ty) {
  switch (ty) {
  case ArrayType::VArray: return "varray";
  case ArrayType::DArray: return "darray";
  case ArrayType::Vec: return "vec";
  case ArrayType::Dict: return "dict";
  case ArrayType::Other: return "other";
  default:
    always_assert(false);
  }
}

const char* srcName(SerializationSite src) {
  switch (src) {
  case SerializationSite::IsDict:             return "is_dict";
  case SerializationSite::IsVec:              return "is_vec";
  case SerializationSite::IsTuple:            return "is_tuple";
  case SerializationSite::IsShape:            return "is_shape";
  case SerializationSite::IsArray:            return "is_array";
  case SerializationSite::FBSerialize:        return "fb_serialize";
  case SerializationSite::FBCompactSerialize: return "fb_compact_serialize";
  case SerializationSite::Gettype:            return "gettype";
  case SerializationSite::Serialize:          return "serialize";
  case SerializationSite::VarExport:          return "var_export";
  case SerializationSite::PrintR:             return "print_r";
  case SerializationSite::JsonEncode:         return "json_encode";
  default:
    always_assert(false);
  }
}

static auto constexpr num_pl_counters =
  static_cast<size_t>(ArrayType::Count) *
  static_cast<size_t>(SerializationSite::Count);
static ServiceData::ExportedTimeSeries* s_provLoggingCounters[num_pl_counters];

InitFiniNode s_initProvLoggingCounters([] {
  for (size_t idx = 0; idx < num_pl_counters; idx++) {
    constexpr size_t numArrayTypes = static_cast<size_t>(ArrayType::Count);
    auto const at = static_cast<ArrayType>(idx % numArrayTypes);
    auto const src = static_cast<SerializationSite>(idx / numArrayTypes);

    s_provLoggingCounters[idx] = ServiceData::createTimeSeries(
      folly::sformat("vm.provlogging.unsampled.{}.{}",
                     arrayTypeName(at),
                     srcName(src)),
      {ServiceData::StatsType::COUNT}
    );
  }
}, InitFiniNode::When::ProcessInit);

} // namespace

void raise_array_serialization_notice(SerializationSite src,
                                      const ArrayData* arr) {
  assertx(RuntimeOption::EvalArrayProvenance);
  if (UNLIKELY(g_context.isNull())) return;
  if (arr->isLegacyArray()) return;
  if (UNLIKELY(g_context->getThrowAllErrors())) {
    throw Exception("Would have logged provenance");
  }
  static auto knownCounter = ServiceData::createTimeSeries(
    "vm.provlogging.known",
    {ServiceData::StatsType::COUNT}
  );
  static decltype(knownCounter) unknownCounters[] = {
    ServiceData::createTimeSeries("vm.provlogging.unknown.counted.nonempty",
                                  {ServiceData::StatsType::COUNT}),
    ServiceData::createTimeSeries("vm.provlogging.unknown.static.nonempty",
                                  {ServiceData::StatsType::COUNT}),
    ServiceData::createTimeSeries("vm.provlogging.unknown.counted.empty",
                                  {ServiceData::StatsType::COUNT}),
    ServiceData::createTimeSeries("vm.provlogging.unknown.static.empty",
                                  {ServiceData::StatsType::COUNT}),
  };

  auto const counterFor = [&] (SerializationSite src, ArrayType at)
    -> ServiceData::ExportedTimeSeries* {
    auto const idx =
      static_cast<size_t>(at) +
      static_cast<size_t>(ArrayType::Count) * static_cast<size_t>(src);

    return s_provLoggingCounters[idx];
  };

  auto const arrayType = [&] {
    if (arr->isVArray()) return ArrayType::VArray;
    if (arr->isDArray()) return ArrayType::DArray;
    if (arr->isVecType()) return ArrayType::Vec;
    if (arr->isDictType()) return ArrayType::Dict;
    return ArrayType::Other;
  }();

  counterFor(src, arrayType)->addValue(1);

  auto const isEmpty = arr->empty();
  auto const isStatic = arr->isStatic();
  auto const isList = !isEmpty && arr->isVectorData();

  static auto const sampl_threshold =
    RAND_MAX / RuntimeOption::EvalLogArrayProvenanceSampleRatio;
  if (std::rand() >= sampl_threshold) return;
  auto const tag = arrprov::getTag(arr);
  if (tag.concrete()) {
    knownCounter->addValue(1);
  } else {
    auto const counterIdx = (isEmpty ? 2 : 0) + (isStatic ? 1 : 0);
    unknownCounters[counterIdx]->addValue(1);
  }

  raise_dynamically_sampled_notice(
      "Observing {}{}{} in {} from {}",
      isStatic ? "static " : "",
      (isEmpty ? "empty " : isList ? "list-like " : "map-like "),
      arrayTypeName(arrayType),
      srcName(src),
      tag.toString());
}

void raise_hack_arr_compat_cast_marked_array_notice(const ArrayData* ad) {
  raise_notice("Hack Array Compat: Casting marked %s to Hack array",
               getDataTypeString(ad->toDataType()).data());
}

namespace {

[[noreturn]]
void raise_func_undefined(const char* prefix, const StringData* name,
                          const Class* cls) {
  if (cls) {
    raise_error("%s undefined method %s::%s()", prefix, cls->name()->data(),
                name->data());
  }
  raise_error("%s undefined function %s()", prefix, name->data());
}

}

void raise_hackarr_compat_is_operator(const char* source, const char* target) {
  raise_notice(
    "Hack Array Compat: is/as operator used with %s and %s",
    source,
    target
  );
}

void raise_resolve_undefined(const StringData* name, const Class* cls) {
  raise_func_undefined("Failure to resolve", name, cls);
}

void raise_call_to_undefined(const StringData* name, const Class* cls) {
  raise_func_undefined("Call to", name, cls);
}

void raise_recoverable_error(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_recoverable_error(msg);
}

static int64_t g_notice_counter = 0;

static bool notice_freq_check(ErrorMode mode) {
  if (!g_context->getThrowAllErrors() &&
      (RuntimeOption::NoticeFrequency <= 0 ||
       g_notice_counter++ % RuntimeOption::NoticeFrequency != 0)) {
    return false;
  }
  return g_context->errorNeedsHandling(
    static_cast<int>(mode), true, ExecutionContext::ErrorThrowMode::Never);
}

#define HANDLE_ERROR(userHandle, throwMode, str, skip)                  \
  g_context->handleError(msg, static_cast<int>(mode), userHandle,       \
                         ExecutionContext::ErrorThrowMode::throwMode,   \
                         str,                                           \
                         skip);

static void raise_notice_helper(ErrorMode mode, bool skipTop,
                                const std::string& msg) {
  switch (mode) {
    case ErrorMode::STRICT:
      HANDLE_ERROR(true, Never, "\nStrict Warning: ", skipTop);
      break;
    case ErrorMode::NOTICE:
      HANDLE_ERROR(true, Never, "\nNotice: ", skipTop);
      break;
    case ErrorMode::PHP_DEPRECATED:
      HANDLE_ERROR(true, Never, "\nDeprecated: ", skipTop);
      break;
    default:
      always_assert(!"Unhandled type of error");
  }
}

void raise_strict_warning(const std::string &msg) {
  if (notice_freq_check(ErrorMode::STRICT)) {
    raise_notice_helper(ErrorMode::STRICT, false, msg);
  }
}

void raise_strict_warning_without_first_frame(const std::string &msg) {
  if (notice_freq_check(ErrorMode::STRICT)) {
    raise_notice_helper(ErrorMode::STRICT, true, msg);
  }
}

void raise_strict_warning(const char *fmt, ...) {
  if (!notice_freq_check(ErrorMode::STRICT)) return;

  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_notice_helper(ErrorMode::STRICT, false, msg);
}

static int64_t g_warning_counter = 0;

bool warning_freq_check() {
  if (!g_context->getThrowAllErrors() &&
      (RuntimeOption::WarningFrequency <= 0 ||
       g_warning_counter++ % RuntimeOption::WarningFrequency != 0)) {
    return false;
  }
  return g_context->errorNeedsHandling(
    static_cast<int>(ErrorMode::WARNING), true,
    ExecutionContext::ErrorThrowMode::Never);
}

void raise_warning_helper(bool skipTop, const std::string& msg) {
  auto mode = ErrorMode::WARNING;
  HANDLE_ERROR(true, Never, "\nWarning: ", skipTop);
}

void raise_warning(const std::string &msg) {
  if (warning_freq_check()) {
    raise_warning_helper(false, msg);
  }
}

void raise_warning_without_first_frame(const std::string &msg) {
  if (warning_freq_check()) {
    raise_warning_helper(true, msg);
  }
}

void raise_warning(const char *fmt, ...) {
  if (!warning_freq_check()) return;
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_warning_helper(false, msg);
}

static void raise_hack_strict_helper(
  HackStrictOption option, const char *ini_setting, const std::string& msg) {
  if (option == HackStrictOption::WARN) {
    raise_warning_helper(
      false, std::string("(hhvm.hack.") + ini_setting + "=warn) " + msg);
  } else if (option == HackStrictOption::ON) {
    raise_error(std::string("(hhvm.hack.") + ini_setting + "=error) " + msg);
  }
}


/**
 * For use with the HackStrictOption settings. This will warn, error, or do
 * nothing depending on what the user chose for the option. The second param
 * should be the ini setting name after "hhvm.hack."
 */
void raise_hack_strict(HackStrictOption option, const char *ini_setting,
                       const std::string& msg) {
  if (option == HackStrictOption::WARN ?
      !warning_freq_check() : (option != HackStrictOption::ON)) {
    return;
  }
  raise_hack_strict_helper(option, ini_setting, msg);
}

void raise_hack_strict(HackStrictOption option, const char *ini_setting,
                       const char *fmt, ...) {
  if (option == HackStrictOption::WARN ?
      !warning_freq_check() : (option != HackStrictOption::ON)) {
    return;
  }
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_hack_strict_helper(option, ini_setting, msg);
}

/**
 * Warnings are currently sampled. raise_warning_unsampled can help when
 * migrating warnings to errors.
 *
 * In general, RaiseDebuggingFrequency should be kept at 1.
 */
static int64_t g_raise_warning_unsampled_counter = 0;

void raise_warning_unsampled(const std::string &msg) {
  if (RuntimeOption::RaiseDebuggingFrequency <= 0 ||
      (g_raise_warning_unsampled_counter++) %
      RuntimeOption::RaiseDebuggingFrequency != 0) {
    return;
  }
  if (g_context->errorNeedsHandling(
        static_cast<int>(ErrorMode::WARNING), true,
        ExecutionContext::ErrorThrowMode::Never)) {
    raise_warning_helper(false, msg);
  }
}

void raise_warning_unsampled(const char *fmt, ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_warning_unsampled(msg);
}

void raise_notice(const std::string &msg) {
  if (notice_freq_check(ErrorMode::NOTICE)) {
    raise_notice_helper(ErrorMode::NOTICE, false, msg);
  }
}

void raise_notice_without_first_frame(const std::string &msg) {
  if (notice_freq_check(ErrorMode::NOTICE)) {
    raise_notice_helper(ErrorMode::NOTICE, true, msg);
  }
}

void raise_notice(const char *fmt, ...) {
  if (!notice_freq_check(ErrorMode::NOTICE)) return;
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_notice_helper(ErrorMode::NOTICE, false, msg);
}

void raise_deprecated(const std::string &msg) {
  if (notice_freq_check(ErrorMode::PHP_DEPRECATED)) {
    raise_notice_helper(ErrorMode::PHP_DEPRECATED, false, msg);
  }
}

void raise_deprecated_without_first_frame(const std::string &msg) {
  if (notice_freq_check(ErrorMode::PHP_DEPRECATED)) {
    raise_notice_helper(ErrorMode::PHP_DEPRECATED, true, msg);
  }
}

void raise_deprecated(const char *fmt, ...) {
  if (!notice_freq_check(ErrorMode::PHP_DEPRECATED)) return;
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_notice_helper(ErrorMode::PHP_DEPRECATED, false, msg);
}

std::string param_type_error_message(
    const char* func_name,
    int param_num,
    const char* expected_type,
    TypedValue actual_value) {

  // slice off fg1_
  if (strncmp(func_name, "fg1_", 4) == 0) {
    func_name += 4;
  } else if (strncmp(func_name, "tg1_", 4) == 0) {
    func_name += 4;
  }
  assertx(param_num > 0);

  auto const isLegacy =
    isArrayLikeType(type(actual_value)) &&
    val(actual_value).parr->isLegacyArray();
  return folly::sformat(
    "{}() expects parameter {} to be {}, {} given",
    func_name,
    param_num,
    expected_type,
    getDataTypeString(type(actual_value), isLegacy).data());
}

void raise_param_type_warning(
    const char* func_name,
    int param_num,
    const char* expected_type,
    TypedValue actual_value) {

  // its ok to do this before munging, because it only looks at the
  // end of the string
  auto is_constructor = is_constructor_name(func_name);
  if (!is_constructor && !warning_freq_check()) return;

  auto msg = param_type_error_message(func_name,
                                      param_num,
                                      expected_type,
                                      actual_value);

  if (is_constructor) {
    SystemLib::throwExceptionObject(msg);
  }

  raise_warning_helper(false, msg);
}

void raise_message(ErrorMode mode,
                   const char *fmt,
                   va_list ap) {
  std::string msg;
  string_vsnprintf(msg, fmt, ap);
  raise_message(mode, false, msg);
}

void raise_message(ErrorMode mode,
                   const char *fmt,
                   ...) {
  std::string msg;
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
  raise_message(mode, false, msg);
}

void raise_message(ErrorMode mode,
                   bool skipTop,
                   const std::string &msg) {
  if (mode == ErrorMode::ERROR) {
    HANDLE_ERROR(false, Always, "\nFatal error: ", skipTop);
    not_reached();
  }

  if (mode == ErrorMode::RECOVERABLE_ERROR) {
    HANDLE_ERROR(true, IfUnhandled, "\nCatchable fatal error: ", skipTop);
    return;
  }

  if (!g_context->errorNeedsHandling(static_cast<int>(mode), true,
                                     ExecutionContext::ErrorThrowMode::Never)) {
    return;
  }

  if (mode == ErrorMode::WARNING) {
    if (RuntimeOption::WarningFrequency <= 0 ||
        (g_warning_counter++) % RuntimeOption::WarningFrequency != 0) {
      return;
    }
    HANDLE_ERROR(true, Never, "\nWarning: ", skipTop);
    return;
  }

  if (RuntimeOption::NoticeFrequency <= 0 ||
      (g_notice_counter++) % RuntimeOption::NoticeFrequency != 0) {
    return;
  }

  raise_notice_helper(mode, skipTop, msg);
}

void raise_str_to_class_notice(const StringData* name) {
  if (RuntimeOption::EvalRaiseStrToClsConversionWarning && !name->isStatic()) {
    raise_notice("Implicit string to Class conversion for classname %s",
                 name->data());
  }
}

void raise_clsmeth_compat_type_hint(
  const Func* func, const std::string& displayName,
  folly::Optional<int> param) {
  if (param) {
    raise_notice(
      "class_meth Compat: Argument %d passed to %s()"
      " must be of type %s, clsmeth given",
      *param + 1, func->fullName()->data(), displayName.c_str());
  } else {
    raise_notice(
      "class_meth Compat: Value returned from function %s()"
      " must be of type %s, clsmeth given",
      func->fullName()->data(), displayName.c_str());
  }
}

void raise_clsmeth_compat_type_hint_outparam_notice(
  const Func* func, const std::string& displayName, int paramNum) {
  raise_notice(
    "class_meth Compat: Argument %d returned from %s()"
    " must be of type %s, clsmeth given",
    paramNum + 1, func->fullName()->data(), displayName.c_str());
}

void raise_clsmeth_compat_type_hint_property_notice(
  const Class* declCls, const StringData* propName,
  const std::string& displayName, bool isStatic) {
  raise_notice(
    "class_meth Compat: %s '%s::%s' declared as type %s, clsmeth assigned",
    isStatic ? "Static property" : "Property",
    declCls->name()->data(), propName->data(), displayName.c_str());
}

///////////////////////////////////////////////////////////////////////////////

#define HC(Opt, ...) \
  SuppressHAC##Opt##Notices::SuppressHAC##Opt##Notices()    \
    : old{RID().getSuppressHAC##Opt##Notices()} {           \
    RID().setSuppressHAC##Opt##Notices(true);               \
  }                                                         \
  SuppressHAC##Opt##Notices::~SuppressHAC##Opt##Notices() { \
    RID().setSuppressHAC##Opt##Notices(old);                \
  }
HAC_CHECK_OPTS
#undef HC

///////////////////////////////////////////////////////////////////////////////

}
