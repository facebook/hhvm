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

#include "hphp/runtime/vm/jit/func-effects.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"

namespace HPHP { namespace jit {

namespace {
const StaticString
  s_http_response_header("http_response_header"),
  s_php_errormsg("php_errormsg"),
  s_extract("extract"),
  s_extractNative("__SystemLib\\extract"),
  s_parse_str("parse_str"),
  s_parse_strNative("__SystemLib\\parse_str"),
  s_assert("assert"),
  s_assertNative("__SystemLib\\assert"),
  s_set_frame_metadata("HH\\set_frame_metadata");

bool funcByNameDestroysLocals(const StringData* fname) {
  if (!fname) return false;
  return fname->isame(s_extract.get()) ||
         fname->isame(s_extractNative.get()) ||
         fname->isame(s_parse_str.get()) ||
         fname->isame(s_parse_strNative.get()) ||
         fname->isame(s_assert.get()) ||
         fname->isame(s_assertNative.get()) ||
         fname->isame(s_set_frame_metadata.get());
}

using FuncSet = std::unordered_set<std::string, string_hashi, string_eqstri>;
/*
 * This is a conservative list of functions that we are certain won't inspect
 * the caller frame (generally by either CallerFrame or vm_call_user_func).
 */
FuncSet ignoresCallerFrame = {
  "array_key_exists",
  "key_exists",
  "array_keys",
  "array_pop",
  "array_push",
  "array_rand",
  "array_search",
  "array_shift",
  "array_slice",
  "array_splice",
  "array_unique",
  "array_unshift",
  "array_values",
  "compact",
  "shuffle",
  "count",
  "sizeof",
  "each",
  "current",
  "in_array",
  "range",
  "sort",
  "rsort",
  "asort",
  "arsort",
  "ksort",
  "krsort",
  "natsort",
  "natcasesort",
  "hphp_array_idx",
  "ctype_alnum",
  "ctype_alpha",
  "ctype_cntrl",
  "ctype_digit",
  "ctype_graph",
  "ctype_lower",
  "ctype_print",
  "ctype_punct",
  "ctype_space",
  "ctype_upper",
  "ctype_xdigit",
  "fb_serialize",
  "fb_unserialize",
  "fb_compact_serialize",
  "fb_compact_unserialize",
  "fb_utf8ize",
  "fb_utf8_strlen",
  "fb_utf8_strlen_deprecated",
  "fb_utf8_substr",
  "fb_get_code_coverage",
  "fb_output_compression",
  "fb_set_exit_callback",
  "fb_get_last_flush_size",
  "fb_lazy_lstat",
  "fb_lazy_realpath",
  "hash",
  "hash_algos",
  "hash_file",
  "hash_final",
  "hash_init",
  "hash_update",
  "hash_copy",
  "hash_equals",
  "furchash_hphp_ext",
  "hphp_murmurhash",
  "get_declared_classes",
  "get_declared_interfaces",
  "get_declared_traits",
  "class_alias",
  "class_exists",
  "interface_exists",
  "trait_exists",
  "enum_exists",
  "get_class_methods",
  "get_class_constants",
  "is_a",
  "is_subclass_of",
  "method_exists",
  "property_exists",
  "error_log",
  "error_reporting",
  "restore_error_handler",
  "restore_exception_handler",
  "set_error_handler",
  "set_exception_handler",
  "hphp_set_error_page",
  "hphp_clear_unflushed",
  "get_defined_functions",
  "function_exists",
  "min",
  "max",
  "abs",
  "is_finite",
  "is_infinite",
  "is_nan",
  "ceil",
  "floor",
  "round",
  "deg2rad",
  "rad2deg",
  "decbin",
  "dechex",
  "decoct",
  "bindec",
  "hexdec",
  "octdec",
  "base_convert",
  "pow",
  "exp",
  "expm1",
  "log10",
  "log1p",
  "log",
  "cos",
  "cosh",
  "sin",
  "sinh",
  "tan",
  "tanh",
  "acos",
  "acosh",
  "asin",
  "asinh",
  "atan",
  "atanh",
  "atan2",
  "hypot",
  "fmod",
  "sqrt",
  "getrandmax",
  "srand",
  "rand",
  "mt_getrandmax",
  "mt_srand",
  "mt_rand",
  "lcg_value",
  "intdiv",
  "flush",
  "hphp_crash_log",
  "hphp_stats",
  "hphp_get_stats",
  "hphp_get_status",
  "hphp_get_iostatus",
  "hphp_set_iostatus_address",
  "hphp_get_timers",
  "hphp_output_global_state",
  "hphp_instruction_counter",
  "hphp_get_hardware_counters",
  "hphp_set_hardware_events",
  "hphp_clear_hardware_events",
  "wordwrap",
  "sprintf",
  "is_null",
  "is_bool",
  "is_int",
  "is_float",
  "is_numeric",
  "is_string",
  "is_scalar",
  "is_array",
  "is_object",
  "is_resource",
  "boolval",
  "intval",
  "floatval",
  "strval",
  "gettype",
  "get_resource_type",
  "settype",
  "serialize",
  "unserialize",
  "addcslashes",
  "stripcslashes",
  "addslashes",
  "stripslashes",
  "bin2hex",
  "hex2bin",
  "nl2br",
  "quotemeta",
  "str_shuffle",
  "strrev",
  "strtolower",
  "strtoupper",
  "ucfirst",
  "lcfirst",
  "ucwords",
  "strip_tags",
  "trim",
  "ltrim",
  "rtrim",
  "chop",
  "explode",
  "implode",
  "join",
  "str_split",
  "chunk_split",
  "strtok",
  "str_replace",
  "str_ireplace",
  "substr_replace",
  "substr",
  "str_pad",
  "str_repeat",
  "html_entity_decode",
  "htmlentities",
  "htmlspecialchars_decode",
  "htmlspecialchars",
  "fb_htmlspecialchars",
  "quoted_printable_encode",
  "quoted_printable_decode",
  "convert_uudecode",
  "convert_uuencode",
  "str_rot13",
  "crc32",
  "crypt",
  "md5",
  "sha1",
  "strtr",
  "convert_cyr_string",
  "get_html_translation_table",
  "hebrev",
  "hebrevc",
  "setlocale",
  "localeconv",
  "nl_langinfo",
  "chr",
  "ord",
  "money_format",
  "number_format",
  "strcmp",
  "strncmp",
  "strnatcmp",
  "strcasecmp",
  "strncasecmp",
  "strnatcasecmp",
  "strcoll",
  "substr_compare",
  "strchr",
  "strrchr",
  "strstr",
  "stristr",
  "strpbrk",
  "strpos",
  "stripos",
  "strrpos",
  "strripos",
  "substr_count",
  "strspn",
  "strcspn",
  "strlen",
  "str_getcsv",
  "count_chars",
  "str_word_count",
  "levenshtein",
  "similar_text",
  "soundex",
  "metaphone",
  "base64_decode",
  "base64_encode",
  "get_headers",
  "get_meta_tags",
  "http_build_query",
  "parse_url",
  "rawurldecode",
  "rawurlencode",
  "urldecode",
  "urlencode",
};

bool funcByNameNeedsCallerFrame(const StringData* fname) {
  return ignoresCallerFrame.find(fname->data()) == ignoresCallerFrame.end();
}
}

bool builtinFuncDestroysLocals(const Func* callee) {
  assertx(callee && callee->isCPPBuiltin());
  auto const fname = callee->name();
  return funcByNameDestroysLocals(fname);
}

bool callDestroysLocals(const NormalizedInstruction& inst,
                        const Func* caller) {
  // We don't handle these two cases, because we don't compile functions
  // containing them:
  assertx(caller->lookupVarId(s_php_errormsg.get()) == -1);
  assertx(caller->lookupVarId(s_http_response_header.get()) == -1);

  auto* unit = caller->unit();
  auto checkTaintId = [&](Id id) {
    auto const str = unit->lookupLitstrId(id);
    return funcByNameDestroysLocals(str);
  };

  if (inst.op() == OpFCallBuiltin) return checkTaintId(inst.imm[2].u_SA);
  if (!isFCallStar(inst.op()))     return false;

  const FPIEnt *fpi = caller->findFPI(inst.source.offset());
  assertx(fpi);
  auto const fpushPc = unit->at(fpi->m_fpushOff);
  auto const op = peek_op(fpushPc);

  if (op == OpFPushFunc) {
    // If the call has any arguments, the FPushFunc will be in a different
    // tracelet -- the tracelet will break on every FPass* because the reffiness
    // of the callee isn't knowable. So we have to say the call destroys locals,
    // to be conservative. If there aren't any arguments, then it can't destroy
    // locals -- even if the call is to extract(), there's no argument, so it
    // won't do anything.
    auto const numArgs = inst.imm[0].u_IVA;
    return (numArgs != 0);
  }
  if (op == OpFPushFuncD) return checkTaintId(getImm(fpushPc, 1).u_SA);
  if (op == OpFPushFuncU) {
    return checkTaintId(getImm(fpushPc, 1).u_SA) ||
           checkTaintId(getImm(fpushPc, 2).u_SA);
  }

  return false;
}

bool builtinFuncNeedsCallerFrame(const Func* callee) {
  assertx(callee && callee->isCPPBuiltin());
  return funcByNameNeedsCallerFrame(callee->name());
}

bool callNeedsCallerFrame(const NormalizedInstruction& inst,
                          const Func* caller) {
  auto* unit = caller->unit();
  auto checkTaintId = [&](Id id) {
    auto const str = unit->lookupLitstrId(id);

    if (!str) return true; // if the function was invoked dynamically we can't
                           // be sure
    /*
     * Only C++ functions can inspect the caller frame, we know these are all
     * loaded ahead of time and unique/persistent.
     */
    if (auto f = Unit::lookupFunc(str)) {
      return f->isCPPBuiltin() && funcByNameNeedsCallerFrame(str);
    }
    return false;
  };
  if (inst.op() == OpFCallBuiltin) return checkTaintId(inst.imm[2].u_SA);
  if (!isFCallStar(inst.op()))     return false;

  const FPIEnt *fpi = caller->findFPI(inst.source.offset());
  assertx(fpi);
  auto const fpushPc = unit->at(fpi->m_fpushOff);
  auto const op = peek_op(fpushPc);

  if (op == OpFPushFunc)  return true;
  if (op == OpFPushFuncD) return checkTaintId(getImm(fpushPc, 1).u_SA);
  if (op == OpFPushFuncU) {
    return checkTaintId(getImm(fpushPc, 1).u_SA) ||
           checkTaintId(getImm(fpushPc, 2).u_SA);
  }

  return false;
}
}}
