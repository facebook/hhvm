/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_options.h>
#include <runtime/ext/ext_function.h>
#include <runtime/ext/extension.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/ini_setting.h>
#include <runtime/base/memory/memory_manager.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/timeout_thread.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/zend/zend_functions.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/eval/runtime/eval_state.h>
#include <util/process.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <system/gen/php/globals/constants.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class OptionData : public RequestEventHandler {
public:
  virtual void requestInit() {
    assertActive = RuntimeOption::AssertActive ? 1 : 0;
    assertWarning = RuntimeOption::AssertWarning ? 1 : 0;
    assertBail = 0;
  }

  virtual void requestShutdown() {
    assertCallback.unset();
  }

  int assertActive;
  int assertWarning;
  int assertBail;
  Variant assertCallback;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(OptionData, s_option_data);

Variant f_assert_options(int what, CVarRef value /* = null_variant */) {
  if (what == k_ASSERT_ACTIVE) {
    int oldValue = s_option_data->assertActive;
    if (!value.isNull()) s_option_data->assertActive = value.toInt64();
    return oldValue;
  }
  if (what == k_ASSERT_WARNING) {
    int oldValue = s_option_data->assertWarning;
    if (!value.isNull()) s_option_data->assertWarning = value.toInt64();
    return oldValue;
  }
  if (what == k_ASSERT_BAIL) {
    int oldValue = s_option_data->assertBail;
    if (!value.isNull()) s_option_data->assertBail = value.toInt64();
    return oldValue;
  }
  if (what == k_ASSERT_CALLBACK) {
    Variant oldValue = s_option_data->assertCallback;
    if (!value.isNull()) s_option_data->assertCallback = value;
    return oldValue;
  }
  throw_invalid_argument("assert option %d is not supported", what);
  return false;
}

Variant f_assert(CVarRef assertion) {
  if (!s_option_data->assertActive) return true;
  if (assertion.isString()) {
    throw NotSupportedException(__func__,
                                "assert cannot take string argument");
  }
  if (assertion.toBoolean()) return true;

  // assertion failed
  if (!s_option_data->assertCallback.isNull()) {
    f_call_user_func(1, s_option_data->assertCallback);
  }

  if (s_option_data->assertWarning) {
    raise_warning("Assertion failed");
  }
  if (s_option_data->assertBail) {
    throw Assertion();
  }
  return null;
}

int f_dl(CStrRef library) {
  return 0;
}

bool f_extension_loaded(CStrRef name) {
  return Extension::IsLoaded(name);
}

Array f_get_loaded_extensions(bool zend_extensions /* = false */) {
  return Extension::GetLoadedExtensions();
}

Array f_get_extension_funcs(CStrRef module_name) {
  throw NotSupportedException(__func__, "extensions are built differently");
}

String f_get_cfg_var(CStrRef option) {
  throw NotSupportedException(__func__, "global configurations not used");
}

String f_get_current_user() {
  int pwbuflen = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (pwbuflen < 1) {
    return "";
  }
  char *pwbuf = (char*)malloc(pwbuflen);
  struct passwd pw;
  struct passwd *retpwptr = NULL;
  if (getpwuid_r(getuid(), &pw, pwbuf, pwbuflen, &retpwptr) != 0) {
    free(pwbuf);
    return "";
  }
  String ret(pw.pw_name, CopyString);
  free(pwbuf);
  return ret;
}

Array f_get_defined_constants(CVarRef categorize /* = null_variant */) {
  if (categorize) {
    throw NotSupportedException(__func__, "constant categorization not "
                                "supported");
  }
  return ClassInfo::GetConstants();
}

String f_get_include_path() {
  return g_context->getIncludePath();
}

void f_restore_include_path() {
}

String f_set_include_path(CStrRef new_include_path) {
  String s = g_context->getIncludePath();
  g_context->setIncludePath(new_include_path);
  return s;
}

Array f_get_included_files() {
  return Eval::RequestEvalState::GetIncludes()["included"];
}

Array f_inclued_get_data() {
  return Eval::RequestEvalState::GetIncludes()["inclued"];
}

int f_get_magic_quotes_gpc() {
  return RuntimeOption::EnableMagicQuotesGpc ? 1 : 0;
}

int f_get_magic_quotes_runtime() {
  throw NotSupportedException(__func__, "not using magic quotes");
}

Array f_get_required_files() {
  throw NotSupportedException(__func__, "requires PHP source code");
}

Variant f_getenv(CStrRef varname) {
  String ret = g_context->getenv(varname);
  if (!ret.isNull()) {
    return ret;
  }
  return false;
}

int f_getlastmod() {
  throw NotSupportedException(__func__, "page modified time not supported");
}

int f_getmygid() {
  return getgid();
}

int f_getmyinode() {
  throw NotSupportedException(__func__, "not exposing operating system info");
}

int f_getmypid() {
  return getpid();
}

int f_getmyuid() {
  return getuid();
}

///////////////////////////////////////////////////////////////////////////////

#define OPTERRCOLON (1)
#define OPTERRNF (2)
#define OPTERRARG (3)

/* Define structure for one recognized option (both single char and long name).
 * If short_open is '-' this is the last option. */
typedef struct _opt_struct {
  char opt_char;
  int  need_param;
  char *opt_name;
} opt_struct;

static int php_opt_error(int argc, char * const *argv, int oint, int optchr,
                         int err, int show_err) {
  if (show_err) {
    fprintf(stderr, "Error in argument %d, char %d: ", oint, optchr+1);
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

static int php_getopt(int argc, char* const *argv, const opt_struct opts[],
                      char **optarg, int *optind, int show_err,
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
                              argv[*optind]+arg_end)) != NULL) {
      arg_end = pos-&argv[*optind][arg_start];
      arg_start++;
    }


    while (1) {
      php_optidx++;
      if (opts[php_optidx].opt_char == '-') {
        (*optind)++;
        return(php_opt_error(argc, argv, *optind-1, optchr, OPTERRARG,
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
      return (php_opt_error(argc, argv, *optind-1, optchr, OPTERRCOLON,
                            show_err));
    }
    arg_start = 1 + optchr;
  }
  if (php_optidx < 0) {
    while (1) {
      php_optidx++;
      if (opts[php_optidx].opt_char == '-') {
        int errind = *optind;
        int errchr = optchr;

        if (!argv[*optind][optchr+1]) {
          dash = 0;
          (*optind)++;
        } else {
          optchr++;
          arg_start++;
        }
        return(php_opt_error(argc, argv, errind, errchr, OPTERRNF, show_err));
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
          return(php_opt_error(argc, argv, *optind-1, optchr, OPTERRARG,
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
  ASSERT(false);
  return(0);  /* never reached */
}

// Free the memory allocated to an longopt array.
static void free_longopts(opt_struct *longopts) {
  opt_struct *p;
  if (longopts) {
    for (p = longopts; p && p->opt_char != '-'; p++) {
      if (p->opt_name != NULL) {
        free(p->opt_name);
      }
    }
  }
}

// Convert the typical getopt input characters to the php_getopt struct array
static int parse_opts(const char * opts, int opts_len, opt_struct **result) {
  int count = 0;
  for (int i = 0; i < opts_len; i++) {
    if ((opts[i] >= 48 && opts[i] <= 57) ||
        (opts[i] >= 65 && opts[i] <= 90) ||
        (opts[i] >= 97 && opts[i] <= 122)) {
      count++;
    }
  }

  opt_struct *paras = (opt_struct *)malloc(sizeof(opt_struct) * count);
  memset(paras, 0, sizeof(opt_struct) * count);
  *result = paras;
  while ((*opts >= 48 && *opts <= 57) ||  /* 0 - 9 */
         (*opts >= 65 && *opts <= 90) ||  /* A - Z */
         (*opts >= 97 && *opts <= 122)) { /* a - z */
    paras->opt_char = *opts;
    paras->need_param = (*(++opts) == ':') ? 1 : 0;
    paras->opt_name = NULL;
    if (paras->need_param == 1) {
      opts++;
      if (*opts == ':') {
        paras->need_param++;
        opts++;
      }
    }
    paras++;
  }
  return count;
}

Array f_getopt(CStrRef options, CVarRef longopts /* = null_variant */) {
  opt_struct *opts, *orig_opts;
  int len = parse_opts(options.data(), options.size(), &opts);

  if (!longopts.isNull()) {
    Array arropts = longopts.toArray();
    int count = arropts.size();

    /* the first <len> slots are filled by the one short ops
     * we now extend our array and jump to the new added structs */
    opts = (opt_struct *)realloc(opts, sizeof(opt_struct) * (len + count + 1));
    orig_opts = opts;
    opts += len;

    memset(opts, 0, count * sizeof(opt_struct));

    for (ArrayIter iter(arropts); iter; ++iter) {
      String entry = iter.second().toString();

      opts->need_param = 0;
      opts->opt_name = strdup(entry.data());
      len = strlen(opts->opt_name);
      if ((len > 0) && (opts->opt_name[len - 1] == ':')) {
        opts->need_param++;
        opts->opt_name[len - 1] = '\0';
        if ((len > 1) && (opts->opt_name[len - 2] == ':')) {
          opts->need_param++;
          opts->opt_name[len - 2] = '\0';
        }
      }
      opts->opt_char = 0;
      opts++;
    }
  } else {
    opts = (opt_struct*) realloc(opts, sizeof(opt_struct) * (len + 1));
    orig_opts = opts;
    opts += len;
  }

  /* php_getopt want to identify the last param */
  opts->opt_char   = '-';
  opts->need_param = 0;
  opts->opt_name   = NULL;

  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  Array vargv = g->GV(argv).toArray();
  int argc = vargv.size();
  char **argv = (char **)malloc((argc+1) * sizeof(char*));
  vector<String> holders;
  int index = 0;
  for (ArrayIter iter(vargv); iter; ++iter) {
    String arg = iter.second().toString();
    holders.push_back(arg);
    argv[index++] = (char*)arg.data();
  }
  argv[index] = NULL;

  Array ret = Array::Create();

  /* after our pointer arithmetic jump back to the first element */
  opts = orig_opts;

  int o;
  char *php_optarg = NULL;
  int php_optind = 1;

  Variant val;
  int optchr = 0;
  int dash = 0; /* have already seen the - */
  char opt[2] = { '\0' };
  char *optname;
  int optname_len = 0;
  int php_optidx;
  while ((o = php_getopt(argc, argv, opts, &php_optarg, &php_optind, 0, 1,
                         optchr, dash, php_optidx))
         != -1) {
    /* Skip unknown arguments. */
    if (o == '?') {
      continue;
    }

    /* Prepare the option character and the argument string. */
    if (o == 0) {
      optname = opts[php_optidx].opt_name;
    } else {
      if (o == 1) {
        o = '-';
      }
      opt[0] = o;
      optname = opt;
    }

    if (php_optarg != NULL) {
      /* keep the arg as binary, since the encoding is not known */
      val = String(php_optarg, CopyString);
    } else {
      val = false;
    }

    /* Add this option / argument pair to the result hash. */
    optname_len = strlen(optname);
    if (!(optname_len > 1 && optname[0] == '0') &&
        is_numeric_string(optname, optname_len, NULL, NULL, 0) ==
        KindOfInt64) {
      /* numeric string */
      int optname_int = atoi(optname);
      if (ret.exists(optname_int)) {
        Variant &e = ret.lvalAt(optname_int);
        if (!e.isArray()) {
          ret.set(optname_int, CREATE_VECTOR2(e, val));
        } else {
          e.append(val);
        }
      } else {
        ret.set(optname_int, val);
      }
    } else {
      /* other strings */
      String key(optname, strlen(optname), CopyString);
      if (ret.exists(key)) {
        Variant &e = ret.lvalAt(key);
        if (!e.isArray()) {
          ret.set(key, CREATE_VECTOR2(e, val));
        } else {
          e.append(val);
        }
      } else {
        ret.set(key, val);
      }
    }

    php_optarg = NULL;
  }

  free_longopts(orig_opts);
  free(orig_opts);
  free(argv);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

#define PHP_RUSAGE_PARA(a) #a, (int64)usg.a
Array f_getrusage(int who /* = 0 */) {
  struct rusage usg;
  memset(&usg, 0, sizeof(struct rusage));

  if (getrusage(who == 1 ? RUSAGE_CHILDREN : RUSAGE_SELF, &usg) == -1) {
    throw SystemCallFailure("getrusage");
  }

  return Array(ArrayInit(17, true).
               set(PHP_RUSAGE_PARA(ru_oublock)).
               set(PHP_RUSAGE_PARA(ru_inblock)).
               set(PHP_RUSAGE_PARA(ru_msgsnd)).
               set(PHP_RUSAGE_PARA(ru_msgrcv)).
               set(PHP_RUSAGE_PARA(ru_maxrss)).
               set(PHP_RUSAGE_PARA(ru_ixrss)).
               set(PHP_RUSAGE_PARA(ru_idrss)).
               set(PHP_RUSAGE_PARA(ru_minflt)).
               set(PHP_RUSAGE_PARA(ru_majflt)).
               set(PHP_RUSAGE_PARA(ru_nsignals)).
               set(PHP_RUSAGE_PARA(ru_nvcsw)).
               set(PHP_RUSAGE_PARA(ru_nivcsw)).
               set(PHP_RUSAGE_PARA(ru_nswap)).
               set(PHP_RUSAGE_PARA(ru_utime.tv_usec)).
               set(PHP_RUSAGE_PARA(ru_utime.tv_sec)).
               set(PHP_RUSAGE_PARA(ru_stime.tv_usec)).
               set(PHP_RUSAGE_PARA(ru_stime.tv_sec)).
               create());
}

bool f_clock_getres(int clk_id, Variant sec, Variant nsec) {
#if defined(__APPLE__)
  throw NotSupportedException(__func__, "feature not supported on OSX");
#else
  struct timespec ts;
  int ret = clock_getres(clk_id, &ts);
  sec = (int64)ts.tv_sec;
  nsec = (int64)ts.tv_nsec;
  return ret == 0;
#endif
}

bool f_clock_gettime(int clk_id, Variant sec, Variant nsec) {
#if defined(__APPLE__)
  throw NotSupportedException(__func__, "feature not supported on OSX");
#else
  struct timespec ts;
  int ret = clock_gettime(clk_id, &ts);
  sec = (int64)ts.tv_sec;
  nsec = (int64)ts.tv_nsec;
  return ret == 0;
#endif
}

bool f_clock_settime(int clk_id, int64 sec, int64 nsec) {
#if defined(__APPLE__)
  throw NotSupportedException(__func__, "feature not supported on OSX");
#else
  struct timespec ts;
  ts.tv_sec = sec;
  ts.tv_nsec = nsec;
  int ret = clock_settime(clk_id, &ts);
  return ret == 0;
#endif
}

String f_ini_alter(CStrRef varname, CStrRef newvalue) {
  throw NotSupportedException(__func__, "not using ini");
}

Array f_ini_get_all(CStrRef extension /* = null_string */) {
  throw NotSupportedException(__func__, "not using ini");
}

String f_ini_get(CStrRef varname) {
  String value("");
  IniSetting::Get(varname, value);
  return value;
}

void f_ini_restore(CStrRef varname) {
}

String f_ini_set(CStrRef varname, CStrRef newvalue) {
  String oldvalue = f_ini_get(varname);
  IniSetting::Set(varname, newvalue);
  return oldvalue;
}

int64 f_memory_get_peak_usage(bool real_usage /* = false */) {
  if (RuntimeOption::EnableMemoryManager) {
    MemoryManager *mm = MemoryManager::TheMemoryManager().getNoCheck();
    const MemoryUsageStats &stats = mm->getStats(true);
    return real_usage ? stats.peakUsage : stats.peakAlloc;
  }
  return (int64)Process::GetProcessRSS(Process::GetProcessId()) * 1024 * 1024;
}

int64 f_memory_get_usage(bool real_usage /* = false */) {
  if (RuntimeOption::EnableMemoryManager) {
    MemoryManager *mm = MemoryManager::TheMemoryManager().getNoCheck();
    const MemoryUsageStats &stats = mm->getStats(true);
    return real_usage ? stats.usage : stats.alloc;
  }
  return (int64)Process::GetProcessRSS(Process::GetProcessId()) * 1024 * 1024;
}

String f_php_ini_scanned_files() {
  throw NotSupportedException(__func__, "not using ini");
}

String f_php_logo_guid() {
  throw NotSupportedException(__func__, "not PHP anymore");
}

String f_php_sapi_name() {
  return RuntimeOption::ExecutionMode;
}

String f_php_uname(CStrRef mode /* = null_string */) {
  String ret;
  struct utsname buf;
  if (uname((struct utsname *)&buf) != -1) {
    if (mode == "s") {
      ret = String(buf.sysname, CopyString);
    } else if (mode == "r") {
      ret = String(buf.release, CopyString);
    } else if (mode == "n") {
      ret = String(buf.nodename, CopyString);
    } else if (mode == "v") {
      ret = String(buf.version, CopyString);
    } else if (mode == "m") {
      ret = String(buf.machine, CopyString);
    } else { /* assume mode == "a" */
      char tmp_uname[512];
      snprintf(tmp_uname, sizeof(tmp_uname), "%s %s %s %s %s",
               buf.sysname, buf.nodename, buf.release, buf.version,
               buf.machine);
      ret = String(tmp_uname, CopyString);
    }
  }
  return ret;
}

bool f_phpcredits(int flag /* = 0 */) {
  throw NotSupportedException(__func__, "not PHP anymore");
}

bool f_phpinfo(int what /* = 0 */) {
  echo("HipHop\n");
  return false;
}

String f_phpversion(CStrRef extension /* = null_string */) {
  return k_PHP_VERSION;
}

bool f_putenv(CStrRef setting) {
  int pos = setting.find('=');
  if (pos >= 0) {
    String name = setting.substr(0, pos);
    String value = setting.substr(pos + 1);
    g_context->setenv(name, value);
    return true;
  }
  return false;
}

bool f_set_magic_quotes_runtime(bool new_setting) {
  if (new_setting) {
    throw NotSupportedException(__func__, "not using magic quotes");
  }
  return true;
}

void f_set_time_limit(int seconds) {
  TimeoutThread::DeferTimeout(seconds);
  // Just for ini_get
  g_context->setRequestTimeLimit(seconds);
}

String f_sys_get_temp_dir() {
  char *env = getenv("TMPDIR");
  if (env && *env) return String(env, CopyString);
  return "/tmp";
}

String f_zend_logo_guid() {
  throw NotSupportedException(__func__, "not zend anymore");
}

int f_zend_thread_id() {
  throw NotSupportedException(__func__, "not zend anymore");
}

String f_zend_version() {
  throw NotSupportedException(__func__, "not zend anymore");
}

///////////////////////////////////////////////////////////////////////////////

#define sign(n) ((n)<0?-1:((n)>0?1:0))

static char *php_canonicalize_version(const char *version) {
  int len = strlen(version);
  char *buf = (char*)malloc(len * 2 + 1), *q, lp, lq;
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

typedef struct {
  const char *name;
  int order;
} special_forms_t;

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
    {NULL, 0},
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
    ver1 = strdup(orig_ver1);
  } else {
    ver1 = php_canonicalize_version(orig_ver1);
  }
  if (orig_ver2[0] == '#') {
    ver2 = strdup(orig_ver2);
  } else {
    ver2 = php_canonicalize_version(orig_ver2);
  }
  p1 = n1 = ver1;
  p2 = n2 = ver2;
  while (*p1 && *p2 && n1 && n2) {
    if ((n1 = strchr(p1, '.')) != NULL) {
      *n1 = '\0';
    }
    if ((n2 = strchr(p2, '.')) != NULL) {
      *n2 = '\0';
    }
    if (isdigit(*p1) && isdigit(*p2)) {
      /* compare element numerically */
      l1 = strtol(p1, NULL, 10);
      l2 = strtol(p2, NULL, 10);
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
    if (n1 != NULL) {
      p1 = n1 + 1;
    }
    if (n2 != NULL) {
      p2 = n2 + 1;
    }
  }
  if (compare == 0) {
    if (n1 != NULL) {
      if (isdigit(*p1)) {
        compare = 1;
      } else {
        compare = php_version_compare(p1, "#N#");
      }
    } else if (n2 != NULL) {
      if (isdigit(*p2)) {
        compare = -1;
      } else {
        compare = php_version_compare("#N#", p2);
      }
    }
  }
  free(ver1);
  free(ver2);
  return compare;
}

Variant f_version_compare(CStrRef version1, CStrRef version2,
                          CStrRef sop /* = null_string */) {
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
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
