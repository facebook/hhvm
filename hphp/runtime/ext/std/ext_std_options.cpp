/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#include "hphp/runtime/ext/std/ext_std_options.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>
#include <vector>

#ifndef _WIN32
#include <sys/utsname.h>
#endif

#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/portability/SysResource.h>
#include <folly/portability/SysTime.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-array-like.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/std/ext_std_misc.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/process.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/timer.h"
#include "hphp/util/user-info.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Linux: /tmp
// MacOS: /var/tmp
const StaticString s_DEFAULT_TEMP_DIR(P_tmpdir);

///////////////////////////////////////////////////////////////////////////////

static bool HHVM_FUNCTION(extension_loaded, const String& name) {
  return ExtensionRegistry::isLoaded(name);
}

static Array
HHVM_FUNCTION(get_loaded_extensions, bool /*zend_extensions*/ /*=false */) {
  return ExtensionRegistry::getLoaded();
}

static TypedValue HHVM_FUNCTION(get_extension_funcs, const String& module_name) {
  auto extension = ExtensionRegistry::get(module_name);
  if (!extension) return make_tv<KindOfBoolean>(false);

  auto const& fns = extension->getExtensionFunctions();
  VecInit result(fns.size());
  for (auto const& fn : fns) {
    result.append(Variant(fn));
  }
  return make_array_like_tv(result.create());
}

static String HHVM_FUNCTION(get_current_user) {
#ifdef _MSC_VER
  return Process::GetCurrentUser();
#else
  auto uid = [] () -> uid_t {
    if (auto cred = get_cli_ucred()) return cred->uid;
    return getuid();
  }();

  auto buf = PasswdBuffer{};
  passwd* pw;
  if (getpwuid_r(uid, &buf.ent, buf.data.get(), buf.size, &pw) != 0) {
    // Unable to lookup the current user.
    return empty_string();
  }
  if (pw == nullptr) {
    // Current user does not exist.
    return empty_string();
  }

  String ret(pw->pw_name, CopyString);
  return ret;
#endif
}

static Array HHVM_FUNCTION(get_defined_constants, bool categorize /*=false */) {
  return lookupDefinedConstants(categorize);
}

static String HHVM_FUNCTION(get_include_path) {
  static StaticString s_include_path("include_path");
  return IniSetting::Get(s_include_path);
}

static void HHVM_FUNCTION(restore_include_path) {
  auto path = RequestInfo::s_requestInfo.getNoCheck()->
    m_reqInjectionData.getDefaultIncludePath();
  IniSetting::SetUser("include_path", path);
}

static String HHVM_FUNCTION(set_include_path, const Variant& new_include_path) {
  String s = HHVM_FN(get_include_path)();
  IniSetting::SetUser("include_path", new_include_path.toString());
  return s;
}

static Array HHVM_FUNCTION(get_included_files) {
  VecInit vai{g_context->m_evaledFilesOrder.size()};
  for (auto& file : g_context->m_evaledFilesOrder) {
    vai.append(Variant{const_cast<StringData*>(file)});
  }
  return vai.toArray();
}

static void HHVM_FUNCTION(record_visited_files) {
  if (g_context->m_visitedFiles.isNull()) {
    g_context->m_visitedFiles = ArrayData::CreateKeyset();
  }
}

static Array HHVM_FUNCTION(get_visited_files) {
  return g_context->m_visitedFiles.isNull()
     ? empty_keyset()
     : g_context->m_visitedFiles;
}

static Variant HHVM_FUNCTION(getenv, const String& varname) {
  String ret = g_context->getenv(varname);
  if (!ret.isNull()) {
    return ret;
  }
  return false;
}

static Variant HHVM_FUNCTION(getlastmod) {
  struct stat s;
  int ret = ::stat(g_context->getContainingFileName()->data(), &s);
  return ret == 0 ? s.st_mtime : false;
}

static Variant HHVM_FUNCTION(getmygid) {
  if (auto cred = get_cli_ucred()) return (int64_t)cred->gid;

  int64_t gid = getgid();
  if (gid < 0) {
    return false;
  }
  return gid;
}

static Variant HHVM_FUNCTION(getmyinode) {
  struct stat s;
  int ret = ::stat(g_context->getContainingFileName()->data(), &s);
  return ret == 0 ? s.st_ino : false;
}

static Variant HHVM_FUNCTION(getmypid) {
  if (auto cred = get_cli_ucred()) return cred->pid;

  int64_t pid = getpid();
  if (pid <= 0) {
    return false;
  }
  return pid;
}

static Variant HHVM_FUNCTION(getmyuid) {
  if (auto cred = get_cli_ucred()) return (int64_t)cred->uid;

  int64_t uid = getuid();
  if (uid < 0) {
    return false;
  }
  return uid;
}

///////////////////////////////////////////////////////////////////////////////

#define OPTERRCOLON (1)
#define OPTERRNF (2)
#define OPTERRARG (3)

/* Define structure for one recognized option (both single char and long name).
 * If short_open is '-' this is the last option. */
struct opt_struct {
  char opt_char{0};
  int  need_param{0};
  char* opt_name{nullptr};
};

static int php_opt_error(const req::vector<char*>& argv, int64_t oint, int optchr,
                         int err, int show_err) {
  if (show_err) {
    fprintf(stderr, "Error in argument %" PRId64 ", char %d: ", oint, optchr+1);
    switch (err) {
    case OPTERRCOLON:
      fprintf(stderr, ": in flags\n");
      break;
    case OPTERRNF:
      fprintf(stderr, "option not found %c\n", argv[oint][optchr]);
      break;
    case OPTERRARG:
      fprintf(stderr, "no argument for option %c\n", argv[oint][optchr]);
      break;
    default:
      fprintf(stderr, "unknown\n");
      break;
    }
  }
  return('?');
}

static int php_getopt(int argc, req::vector<char*>& argv,
                      req::vector<opt_struct>& opts,
                      char **optarg, int64_t *optind, int show_err,
                      int arg_start, int &optchr, int &dash, int &php_optidx) {
  php_optidx = -1;

  if (*optind >= argc) {
    return(EOF);
  }
  if (!dash) {
    if ((argv[*optind][0] !=  '-')) {
      return(EOF);
    } else {
      if (!argv[*optind][1]) {
        /*
        * use to specify stdin. Need to let pgm process this and
        * the following args
        */
        return(EOF);
      }
    }
  }
  if ((argv[*optind][0] == '-') && (argv[*optind][1] == '-')) {
    const char *pos;
    int arg_end = strlen(argv[*optind])-1;

    // '--' indicates end of args if not followed by a known long option name
    if (argv[*optind][2] == '\0') {
      (*optind)++;
      return(EOF);
    }

    arg_start = 2;

    /* Check for <arg>=<val> */
    if ((pos = string_memnstr(&argv[*optind][arg_start], "=", 1,
                              argv[*optind]+arg_end)) != nullptr) {
      arg_end = pos-&argv[*optind][arg_start];
      arg_start++;
    }


    while (1) {
      php_optidx++;
      if (opts[php_optidx].opt_char == '-') {
        (*optind)++;
        return(php_opt_error(argv, *optind-1, optchr, OPTERRARG,
                             show_err));
      } else if (opts[php_optidx].opt_name &&
                 !strncmp(&argv[*optind][2], opts[php_optidx].opt_name,
                          arg_end)) {
        break;
      }
    }
    optchr = 0;
    dash = 0;
    arg_start += strlen(opts[php_optidx].opt_name);
  } else {
    if (!dash) {
      dash = 1;
      optchr = 1;
    }
    /* Check if the guy tries to do a -: kind of flag */
    if (argv[*optind][optchr] == ':') {
      dash = 0;
      (*optind)++;
      return (php_opt_error(argv, *optind-1, optchr, OPTERRCOLON,
                            show_err));
    }
    arg_start = 1 + optchr;
  }
  if (php_optidx < 0) {
    while (1) {
      php_optidx++;
      if (opts[php_optidx].opt_char == '-') {
        int64_t errind = *optind;
        int errchr = optchr;

        if (!argv[*optind][optchr+1]) {
          dash = 0;
          (*optind)++;
        } else {
          optchr++;
          arg_start++;
        }
        return(php_opt_error(argv, errind, errchr, OPTERRNF, show_err));
      } else if (argv[*optind][optchr] == opts[php_optidx].opt_char) {
        break;
      }
    }
  }
  if (opts[php_optidx].need_param) {
    /* Check for cases where the value of the argument
    is in the form -<arg> <val>, -<arg>=<varl> or -<arg><val> */
    dash = 0;
    if (!argv[*optind][arg_start]) {
      (*optind)++;
      if (*optind == argc) {
        /* Was the value required or is it optional? */
        if (opts[php_optidx].need_param == 1) {
          return(php_opt_error(argv, *optind-1, optchr, OPTERRARG,
                               show_err));
        }
      /* Optional value is not supported with -<arg> <val> style */
      } else if (opts[php_optidx].need_param == 1) {
        *optarg = argv[(*optind)++];
       }
    } else if (argv[*optind][arg_start] == '=') {
      arg_start++;
      *optarg = &argv[*optind][arg_start];
      (*optind)++;
    } else {
      *optarg = &argv[*optind][arg_start];
      (*optind)++;
    }
    return opts[php_optidx].opt_char;
  } else {
    /* multiple options specified as one (exclude long opts) */
    if (arg_start >= 2 && !((argv[*optind][0] == '-') &&
                            (argv[*optind][1] == '-'))) {
      if (!argv[*optind][optchr+1])
      {
        dash = 0;
        (*optind)++;
      } else {
        optchr++;
      }
    } else {
      (*optind)++;
    }
    return opts[php_optidx].opt_char;
  }
  assertx(false);
  return(0);  /* never reached */
}

// Free the memory allocated to an longopt array.
static void free_longopts(req::vector<opt_struct>& longopts) {
  for (auto& p : longopts) {
    if (p.opt_char == '-') break;
    req::free(p.opt_name);
  }
}

// Convert the typical getopt input characters to the php_getopt struct array
static req::vector<opt_struct> parse_opts(const char *opts, int opts_len) {
  int count = 0;
  for (int i = 0; i < opts_len; i++) {
    if ((opts[i] >= 48 && opts[i] <= 57) ||
        (opts[i] >= 65 && opts[i] <= 90) ||
        (opts[i] >= 97 && opts[i] <= 122)) {
      count++;
    }
  }
  req::vector<opt_struct> paras(count);
  int i = 0;
  while ((*opts >= 48 && *opts <= 57) ||  /* 0 - 9 */
         (*opts >= 65 && *opts <= 90) ||  /* A - Z */
         (*opts >= 97 && *opts <= 122)) { /* a - z */
    auto& p = paras[i];
    p.opt_char = *opts;
    p.need_param = (*(++opts) == ':') ? 1 : 0;
    p.opt_name = nullptr;
    if (p.need_param == 1) {
      opts++;
      if (*opts == ':') {
        p.need_param++;
        opts++;
      }
    }
    ++i;
  }
  assertx(i == count);
  return paras;
}

const StaticString s_argv("argv");

static Array HHVM_FUNCTION(getopt_with_optind,
                           const String& options,
                           const Variant& longopts,
                           int64_t& optind) {
  if (optind <= 0) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Parameter optind must be a positive integer"
    );
  }

  auto opt_vec = parse_opts(options.data(), options.size());

  if (!longopts.isNull()) {
    Array arropts = longopts.toArray();

    /* the first vec.size() slots are filled by the one short ops
     * we now extend our array and jump to the new added structs */
    auto i = opt_vec.size();
    opt_vec.resize(opt_vec.size() + arropts.size() + 1);

    for (ArrayIter iter(arropts); iter; ++iter) {
      String entry = iter.second().toString();

      auto& opt = opt_vec[i];
      opt.need_param = 0;
      opt.opt_name = req::strdup(entry.data());
      auto len = strlen(opt.opt_name);
      if ((len > 0) && (opt.opt_name[len - 1] == ':')) {
        opt.need_param++;
        opt.opt_name[len - 1] = '\0';
        if ((len > 1) && (opt.opt_name[len - 2] == ':')) {
          opt.need_param++;
          opt.opt_name[len - 2] = '\0';
        }
      }
      opt.opt_char = 0;
      ++i;
    }
    assertx(i == opt_vec.size() - 1);
  } else {
    opt_vec.resize(opt_vec.size() + 1);
  }

  /* php_getopt want to identify the last param */
  auto& last = opt_vec[opt_vec.size() - 1];
  last.opt_char   = '-';
  last.need_param = 0;
  last.opt_name   = nullptr;

  auto const& vargv = php_global(s_argv).toArray();
  int argc = vargv.size();
  req::vector<char*> argv(argc + 1);
  req::vector<String> holders;
  holders.reserve(argc);
  int index = 0;
  for (ArrayIter iter(vargv); iter; ++iter) {
    String arg = iter.second().toString();
    holders.push_back(arg);
    argv[index++] = (char*)arg.data();
  }
  argv[index] = nullptr;

  int o;
  char *php_optarg = nullptr;
  int64_t php_optind = optind;

  SCOPE_EXIT {
    free_longopts(opt_vec);
  };

  Array ret = Array::CreateDict();

  Variant val;
  int optchr = 0;
  int dash = 0; /* have already seen the - */
  char opt[2] = { '\0' };
  char *optname;
  int optname_len = 0;
  int php_optidx;
  while ((o = php_getopt(argc, argv, opt_vec, &php_optarg, &php_optind, 0, 1,
                         optchr, dash, php_optidx))
         != -1) {
    /* Skip unknown arguments. */
    if (o == '?') {
      continue;
    }

    /* Prepare the option character and the argument string. */
    if (o == 0) {
      optname = opt_vec[php_optidx].opt_name;
    } else {
      if (o == 1) {
        o = '-';
      }
      opt[0] = o;
      optname = opt;
    }

    if (php_optarg != nullptr) {
      /* keep the arg as binary, since the encoding is not known */
      val = String(php_optarg, CopyString);
    } else {
      val = false;
    }

    /* Add this option / argument pair to the result hash. */
    optname_len = strlen(optname);
    if (!(optname_len > 1 && optname[0] == '0') &&
        is_numeric_string(optname, optname_len, nullptr, nullptr, 0) ==
        KindOfInt64) {
      /* numeric string */
      int optname_int = atoi(optname);
      if (ret.exists(optname_int)) {
        auto const lval = ret.lval(optname_int);
        if (!isArrayLikeType(lval.type())) {
          ret.set(optname_int, make_vec_array(Variant::wrap(lval.tv()), val));
        } else {
          asArrRef(lval).append(val);
        }
      } else {
        ret.set(optname_int, val);
      }
    } else {
      /* other strings */
      String key(optname, strlen(optname), CopyString);
      if (ret.exists(key)) {
        auto const lval = ret.lval(key);
        if (!isArrayLikeType(lval.type())) {
          ret.set(key, make_vec_array(Variant::wrap(lval.tv()), val));
        } else {
          asArrRef(lval).append(val);
        }
      } else {
        ret.set(key, val);
      }
    }

    php_optarg = nullptr;
    optind = php_optind;
  }

  return ret;
}

static Array HHVM_FUNCTION(getopt, const String& options,
                                   const Variant& longopts /*=null */) {
  int64_t optind = 1;
  return HHVM_FN(getopt_with_optind)(options, longopts, optind);
}

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_ru_oublock("ru_oublock"),
  s_ru_inblock("ru_inblock"),
  s_ru_msgsnd("ru_msgsnd"),
  s_ru_msgrcv("ru_msgrcv"),
  s_ru_maxrss("ru_maxrss"),
  s_ru_ixrss("ru_ixrss"),
  s_ru_idrss("ru_idrss"),
  s_ru_minflt("ru_minflt"),
  s_ru_majflt("ru_majflt"),
  s_ru_nsignals("ru_nsignals"),
  s_ru_nvcsw("ru_nvcsw"),
  s_ru_nivcsw("ru_nivcsw"),
  s_ru_nswap("ru_nswap"),
  s_ru_utime_tv_usec("ru_utime.tv_usec"),
  s_ru_utime_tv_sec("ru_utime.tv_sec"),
  s_ru_stime_tv_usec("ru_stime.tv_usec"),
  s_ru_stime_tv_sec("ru_stime.tv_sec");

#define PHP_RUSAGE_PARA(a) s_ ## a, (int64_t)usg.a
static Array HHVM_FUNCTION(getrusage, int64_t who /* = 0 */) {
  struct rusage usg;
  memset(&usg, 0, sizeof(struct rusage));
  int actual_who;
  switch (who) {
  case 1:
    actual_who = RUSAGE_CHILDREN;
    break;
  case 2:
#ifdef RUSAGE_THREAD
    actual_who = RUSAGE_THREAD;
#else
    throw_not_supported(__func__, "RUSAGE_THREAD is not defined on this sytem");
#endif
    break;
  default:
    actual_who = RUSAGE_SELF;
    break;
  }

  if (getrusage(actual_who, &usg) == -1) {
    raise_error("getrusage returned %d: %s", errno,
      folly::errnoStr(errno).c_str());
  }

  return make_dict_array(
    PHP_RUSAGE_PARA(ru_oublock),
    PHP_RUSAGE_PARA(ru_inblock),
    PHP_RUSAGE_PARA(ru_msgsnd),
    PHP_RUSAGE_PARA(ru_msgrcv),
    PHP_RUSAGE_PARA(ru_maxrss),
    PHP_RUSAGE_PARA(ru_ixrss),
    PHP_RUSAGE_PARA(ru_idrss),
    PHP_RUSAGE_PARA(ru_minflt),
    PHP_RUSAGE_PARA(ru_majflt),
    PHP_RUSAGE_PARA(ru_nsignals),
    PHP_RUSAGE_PARA(ru_nvcsw),
    PHP_RUSAGE_PARA(ru_nivcsw),
    PHP_RUSAGE_PARA(ru_nswap),
    s_ru_utime_tv_usec, (int64_t)usg.ru_utime.tv_usec,
    s_ru_utime_tv_sec,  (int64_t)usg.ru_utime.tv_sec,
    s_ru_stime_tv_usec, (int64_t)usg.ru_stime.tv_usec,
    s_ru_stime_tv_sec,  (int64_t)usg.ru_stime.tv_sec
  );
}

static bool HHVM_FUNCTION(clock_getres,
                          int64_t clk_id, int64_t& sec, int64_t& nsec) {
  struct timespec ts;
  int ret = clock_getres(clk_id, &ts);
  sec = ts.tv_sec;
  nsec = ts.tv_nsec;
  return ret == 0;
}

static bool HHVM_FUNCTION(clock_gettime,
                          int64_t clk_id, int64_t& sec, int64_t& nsec) {
  struct timespec ts;
  int ret = gettime(clockid_t(clk_id), &ts);
  sec = ts.tv_sec;
  nsec = ts.tv_nsec;
  return ret == 0;
}

static int64_t HHVM_FUNCTION(clock_gettime_ns, int64_t clk_id) {
  return gettime_ns(clockid_t(clk_id));
}

static int64_t HHVM_FUNCTION(cpu_get_count) {
  return Process::GetCPUCount();
}

static String HHVM_FUNCTION(cpu_get_model) {
  return Process::GetCPUModel();
}

Variant HHVM_FUNCTION(ini_get, const String& varname) {
  Variant value;
  bool ret = IniSetting::Get(varname, value);

  // ret will be false if varname isn't a valid ini setting
  if (!ret) {
    return false;
  }

  // Anything other than array for ini_get can be converted to
  // the expected string result for this function call.
  // If value is null, an empty string is returned, which is good
  // and expected.
  if (!value.isArray()) {
    return value.toString();
  }
  // For arrays, this will return an array of values. It will also return
  // an empty array if the values for a valid collection-like configuration
  // has not been set. This is similar to returning an empty-string for
  // standard configuration options.
  return value;
}

static ArrayRet HHVM_FUNCTION(ini_get_all,
                           const String& extension, bool detailed) {
  return IniSetting::GetAll(extension, detailed);
}

static void HHVM_FUNCTION(ini_restore, const String& varname) {
  IniSetting::RestoreUser(varname);
}

Variant HHVM_FUNCTION(ini_set,
                      const String& varname, const Variant& newvalue) {
  auto oldvalue = HHVM_FN(ini_get)(varname);
  auto ret = IniSetting::SetUser(varname, newvalue);
  if (!ret) {
    return false;
  }
  return oldvalue;
}

static int64_t HHVM_FUNCTION(memory_get_allocation) {
  auto total = tl_heap->getStatsCopy().totalAlloc;
  assertx(total >= 0);
  return total;
}

static int64_t HHVM_FUNCTION(hphp_memory_get_interval_peak_usage,
                             bool real_usage /*=false */) {
  auto const stats = tl_heap->getStatsCopy();
  int64_t ret = real_usage ? stats.peakIntervalUsage :
                stats.peakIntervalCap;
  assertx(ret >= 0);
  return ret;
}

static int64_t HHVM_FUNCTION(memory_get_peak_usage,
                             bool real_usage /*=false */) {
  auto const stats = tl_heap->getStatsCopy();
  int64_t ret = real_usage ? stats.peakUsage : stats.peakCap;
  assertx(ret >= 0);
  return ret;
}

static int64_t HHVM_FUNCTION(memory_get_usage, bool real_usage /*=false */) {
  auto const stats = tl_heap->getStatsCopy();
  int64_t ret = real_usage ? stats.usage() : stats.capacity();
  // Since we don't always alloc and dealloc a shared structure from the same
  // thread it is possible that this can go negative when we are tracking
  // jemalloc stats.
  assertx((use_jemalloc && real_usage) || ret >= 0);
  return std::max<int64_t>(ret, 0);
}

static int64_t HHVM_FUNCTION(hphp_memory_heap_usage) {
  // This corresponds to PHP memory_get_usage(false), only counting
  // allocations via MemoryManager.
  return tl_heap->getStatsCopy().mmUsage();
}

static int64_t HHVM_FUNCTION(hphp_memory_heap_capacity) {
  // This happens to match HHVM memory_get_usage(false), and
  // PHP memory_get_usage(true).
  return tl_heap->getStatsCopy().capacity();
}

static bool HHVM_FUNCTION(hphp_memory_start_interval) {
  return tl_heap->startStatsInterval();
}

static bool HHVM_FUNCTION(hphp_memory_stop_interval) {
  return tl_heap->stopStatsInterval();
}

const StaticString s_srv("srv"), s_cli("cli");

String HHVM_FUNCTION(php_sapi_name) {
  return is_any_cli_mode() ? s_cli : s_srv;
}

#ifdef _WIN32
const char* php_get_edition_name(DWORD majVer, DWORD minVer);
Optional<String> php_get_windows_name();
String php_get_windows_cpu();
#endif

const StaticString
  s_s("s"),
  s_r("r"),
  s_n("n"),
  s_v("v"),
  s_m("m");

Variant HHVM_FUNCTION(php_uname, const String& mode /*="" */) {
#ifdef _WIN32
  if (mode == s_s) {
    return s_Windows_NT;
  } else if (mode == s_r) {
    DWORD dwVersion = GetVersion();
    DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    DWORD dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
    return folly::sformat("{}.{}", dwMajorVersion, dwMinorVersion);
  } else if (mode == s_n) {
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    char ComputerName[MAX_COMPUTERNAME_LENGTH + 1];

    GetComputerName(ComputerName, &dwSize);
    return String(ComputerName, dwSize, CopyString);
  } else if (mode == s_v) {
    DWORD dwVersion = GetVersion();
    auto dwBuild = (DWORD)(HIWORD(dwVersion));
    auto winVer = php_get_windows_name();
    if (!winVer.hasValue()) {
      return folly::sformat("build {}", dwBuild);
    }
    return folly::sformat("build {} ({})", dwBuild, winVer.value());
  } else if (mode == s_m) {
    return php_get_windows_cpu();
  } else {
    auto winVer = php_get_windows_name();

    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    char ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    GetComputerName(ComputerName, &dwSize);

    DWORD dwVersion = GetVersion();
    DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    DWORD dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
    DWORD dwBuild = (DWORD)(HIWORD(dwVersion));

    if (dwMajorVersion == 6 && dwMinorVersion == 2 && winVer.hasValue()) {
      if (strncmp(winVer.value().c_str(), "Windows 8.1", 11) == 0 ||
          strncmp(winVer.value().c_str(), "Windows Server 2012 R2", 22) == 0) {
        dwMinorVersion = 3;
      }
    }

    return folly::sformat("Windows NT {} {}.{} build {} ({}) {}",
      ComputerName,
      dwMajorVersion,
      dwMinorVersion,
      dwBuild,
      winVer.hasValue() ? winVer.value().c_str() : "unknown",
      php_get_windows_cpu()
    );
  }
#else
  struct utsname buf;
  if (uname((struct utsname *)&buf) == -1) {
    return init_null();
  }

  if (mode == s_s) {
    return String(buf.sysname, CopyString);
  } else if (mode == s_r) {
    return String(buf.release, CopyString);
  } else if (mode == s_n) {
    return String(buf.nodename, CopyString);
  } else if (mode == s_v) {
    return String(buf.version, CopyString);
  } else if (mode == s_m) {
    return String(buf.machine, CopyString);
  } else { /* assume mode == "a" */
    char tmp_uname[512];
    snprintf(tmp_uname, sizeof(tmp_uname), "%s %s %s %s %s",
             buf.sysname, buf.nodename, buf.release, buf.version,
             buf.machine);
    return String(tmp_uname, CopyString);
  }
#endif
}

static Variant HHVM_FUNCTION(phpversion, const String& extension /*="" */) {
  Extension *ext;

  if (extension.empty()) {
    return get_PHP_VERSION();
  }

  if ((ext = ExtensionRegistry::get(extension)) != nullptr &&
      strcmp(ext->getVersion(), NO_EXTENSION_VERSION_YET) != 0) {
    return ext->getVersion();
  }

  return false;
}

static bool HHVM_FUNCTION(putenv, const String& setting) {
  int pos = setting.find('=');
  if (pos >= 0) {
    String name = setting.substr(0, pos);
    String value = setting.substr(pos + 1);
    g_context->setenv(name, value);
  } else {
    g_context->unsetenv(setting);
  }
  return true;
}

static void HHVM_FUNCTION(set_time_limit, int64_t seconds) {
  RequestInfo *info = RequestInfo::s_requestInfo.getNoCheck();
  RequestInjectionData &data = info->m_reqInjectionData;
  if (RuntimeOption::TimeoutsUseWallTime) {
    data.setTimeout(seconds);
  } else {
    data.setCPUTimeout(seconds);
  }
}

static void HHVM_FUNCTION(set_pre_timeout_handler,
  int64_t seconds,
  const Variant& callback
) {
  auto& req_data = RID();
  if (callback.isNull() || !callback.isObject()) {
    // We use 0 to signify no callback
    req_data.setUserTimeout(0);
  } else {
    req_data.setUserTimeout(seconds);
  }
  g_context->m_timeThresholdCallback = callback;
}

String HHVM_FUNCTION(sys_get_temp_dir) {
#ifdef WIN32
  char buf[PATH_MAX];
  auto len = GetTempPathA(PATH_MAX, buf);
  return String(buf, len, CopyString);
#else
  char *env = getenv("TMPDIR");
  if (env && *env) return String(env, CopyString);
  return s_DEFAULT_TEMP_DIR;
#endif
}


///////////////////////////////////////////////////////////////////////////////

#define sign(n) ((n)<0?-1:((n)>0?1:0))

static char *php_canonicalize_version(const char *version) {
  int len = strlen(version);
  char *buf = (char*)req::malloc_noptrs(len * 2 + 1), *q, lp, lq;
  const char *p;

  if (len == 0) {
    *buf = '\0';
    return buf;
  }

  p = version;
  q = buf;
  *q++ = lp = *p++;
  lq = '\0';
  while (*p) {
    /*  s/[-_+]/./g;
     *  s/([^\d\.])([^\D\.])/$1.$2/g;
     *  s/([^\D\.])([^\d\.])/$1.$2/g;
     */
#define isdig(x) (isdigit(x)&&(x)!='.')
#define isndig(x) (!isdigit(x)&&(x)!='.')
#define isspecialver(x) ((x)=='-'||(x)=='_'||(x)=='+')

    lq = *(q - 1);
    if (isspecialver(*p)) {
      if (lq != '.') {
        lq = *q++ = '.';
      }
    } else if ((isndig(lp) && isdig(*p)) || (isdig(lp) && isndig(*p))) {
      if (lq != '.') {
        *q++ = '.';
      }
      lq = *q++ = *p;
    } else if (!isalnum(*p)) {
      if (lq != '.') {
        lq = *q++ = '.';
      }
    } else {
      lq = *q++ = *p;
    }
    lp = *p++;
  }
  *q++ = '\0';
  return buf;
}

struct special_forms_t {
  const char *name;
  int order;
};

static int compare_special_version_forms(const char *form1, const char *form2) {
  int found1 = -1, found2 = -1;
  special_forms_t special_forms[11] = {
    {"dev", 0},
    {"alpha", 1},
    {"a", 1},
    {"beta", 2},
    {"b", 2},
    {"RC", 3},
    {"rc", 3},
    {"#", 4},
    {"pl", 5},
    {"p", 5},
    {nullptr, 0},
  };
  special_forms_t *pp;

  for (pp = special_forms; pp && pp->name; pp++) {
    if (strncmp(form1, pp->name, strlen(pp->name)) == 0) {
      found1 = pp->order;
      break;
    }
  }
  for (pp = special_forms; pp && pp->name; pp++) {
    if (strncmp(form2, pp->name, strlen(pp->name)) == 0) {
      found2 = pp->order;
      break;
    }
  }
  return sign(found1 - found2);
}

static int php_version_compare(const char *orig_ver1, const char *orig_ver2) {
  char *ver1;
  char *ver2;
  char *p1, *p2, *n1, *n2;
  long l1, l2;
  int compare = 0;

  if (!*orig_ver1 || !*orig_ver2) {
    if (!*orig_ver1 && !*orig_ver2) {
      return 0;
    } else {
      return *orig_ver1 ? 1 : -1;
    }
  }
  if (orig_ver1[0] == '#') {
    ver1 = req::strdup(orig_ver1);
  } else {
    ver1 = php_canonicalize_version(orig_ver1);
  }
  SCOPE_EXIT { req::free(ver1); };
  if (orig_ver2[0] == '#') {
    ver2 = req::strdup(orig_ver2);
  } else {
    ver2 = php_canonicalize_version(orig_ver2);
  }
  SCOPE_EXIT { req::free(ver2); };
  p1 = n1 = ver1;
  p2 = n2 = ver2;
  while (*p1 && *p2 && n1 && n2) {
    if ((n1 = strchr(p1, '.')) != nullptr) {
      *n1 = '\0';
    }
    if ((n2 = strchr(p2, '.')) != nullptr) {
      *n2 = '\0';
    }
    if (isdigit(*p1) && isdigit(*p2)) {
      /* compare element numerically */
      l1 = strtol(p1, nullptr, 10);
      l2 = strtol(p2, nullptr, 10);
      compare = sign(l1 - l2);
    } else if (!isdigit(*p1) && !isdigit(*p2)) {
      /* compare element names */
      compare = compare_special_version_forms(p1, p2);
    } else {
      /* mix of names and numbers */
      if (isdigit(*p1)) {
        compare = compare_special_version_forms("#N#", p2);
      } else {
        compare = compare_special_version_forms(p1, "#N#");
      }
    }
    if (compare != 0) {
      break;
    }
    if (n1 != nullptr) {
      p1 = n1 + 1;
    }
    if (n2 != nullptr) {
      p2 = n2 + 1;
    }
  }
  if (compare == 0) {
    if (n1 != nullptr) {
      if (isdigit(*p1)) {
        compare = 1;
      } else {
        compare = php_version_compare(p1, "#N#");
      }
    } else if (n2 != nullptr) {
      if (isdigit(*p2)) {
        compare = -1;
      } else {
        compare = php_version_compare("#N#", p2);
      }
    }
  }
  return compare;
}

Variant HHVM_FUNCTION(version_compare,
                      const String& version1,
                      const String& version2,
                      const String& sop /*="" */) {
  const char *op = sop.data();
  int op_len = sop.size();
  int compare = php_version_compare(version1.data(), version2.data());
  if (sop.empty()) {
    return compare;
  }
  if (!strncmp(op, "<", op_len) || !strncmp(op, "lt", op_len)) {
    return compare == -1;
  }
  if (!strncmp(op, "<=", op_len) || !strncmp(op, "le", op_len)) {
    return compare != 1;
  }
  if (!strncmp(op, ">", op_len) || !strncmp(op, "gt", op_len)) {
    return compare == 1;
  }
  if (!strncmp(op, ">=", op_len) || !strncmp(op, "ge", op_len)) {
    return compare != -1;
  }
  if (!strncmp(op, "==", op_len) || !strncmp(op, "=", op_len) ||
      !strncmp(op, "eq", op_len)) {
    return compare == 0;
  }
  if (!strncmp(op, "!=", op_len) || !strncmp(op, "<>", op_len) ||
      !strncmp(op, "ne", op_len)) {
    return compare != 0;
  }
  return init_null();
}

///////////////////////////////////////////////////////////////////////////////

void StandardExtension::registerNativeOptions() {
  HHVM_FE(extension_loaded);
  HHVM_FE(get_loaded_extensions);
  HHVM_FE(get_extension_funcs);
  HHVM_FE(get_current_user);
  HHVM_FE(get_defined_constants);
  HHVM_FE(get_include_path);
  HHVM_FE(restore_include_path);
  HHVM_FE(set_include_path);
  HHVM_FE(get_included_files);
  HHVM_FE(record_visited_files);
  HHVM_FE(get_visited_files);
  HHVM_FE(getenv);
  HHVM_FE(getlastmod);
  HHVM_FE(getmygid);
  HHVM_FE(getmyinode);
  HHVM_FE(getmypid);
  HHVM_FE(getmyuid);
  HHVM_FE(getopt);
  HHVM_FE(getopt_with_optind);
  HHVM_FE(getrusage);
  HHVM_FE(clock_getres);
  HHVM_FE(clock_gettime);
  HHVM_FE(clock_gettime_ns);
  HHVM_FE(cpu_get_count);
  HHVM_FE(cpu_get_model);
  HHVM_FALIAS(ini_alter, ini_set);
  HHVM_FE(ini_get);
  HHVM_FE(ini_get_all);
  HHVM_FE(ini_restore);
  HHVM_FE(ini_set);
  HHVM_FE(memory_get_peak_usage);
  HHVM_FE(memory_get_usage);
  // HHVM-specific stateless accessors for memory stats
  HHVM_FE(hphp_memory_heap_usage);
  HHVM_FE(hphp_memory_heap_capacity);
  // This is HH-specific as well but code depends on the old name.
  HHVM_FE(memory_get_allocation);
  HHVM_FE(hphp_memory_get_interval_peak_usage);
  HHVM_FE(hphp_memory_start_interval);
  HHVM_FE(hphp_memory_stop_interval);
  HHVM_FE(php_sapi_name);
  HHVM_FE(php_uname);
  HHVM_FE(phpversion);
  HHVM_FE(putenv);
  HHVM_FE(set_time_limit);
  HHVM_FE(set_pre_timeout_handler);
  HHVM_FE(sys_get_temp_dir);
  HHVM_FE(version_compare);

#ifdef CLOCK_REALTIME
  HHVM_RC_INT_SAME(CLOCK_REALTIME);
#endif
#ifdef CLOCK_MONOTONIC
  HHVM_RC_INT_SAME(CLOCK_MONOTONIC);
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
  HHVM_RC_INT_SAME(CLOCK_PROCESS_CPUTIME_ID);
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
  HHVM_RC_INT_SAME(CLOCK_THREAD_CPUTIME_ID);
#endif
}

///////////////////////////////////////////////////////////////////////////////
}
