/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <cpp/ext/ext_options.h>
#include <cpp/ext/ext_function.h>
#include <cpp/base/runtime_option.h>
#include <cpp/base/memory/memory_manager.h>
#include <cpp/base/util/request_local.h>
#include <cpp/base/timeout_thread.h>
#include <util/process.h>
#include <util/logger.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <lib/system/gen/php/globals/constants.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class OptionData : public RequestEventHandler {
public:
  virtual void requestInit() {
    assertActive = RuntimeOption::AssertActive ? 1 : 0;
    assertWarning = RuntimeOption::AssertWarning ? 1 : 0;
    assertBail = 0;
  }

  virtual void requestShutdown() { }

  int assertActive;
  int assertWarning;
  int assertBail;
  Variant assertCallback;
};

static RequestLocal<OptionData> s_option_data;

Variant f_assert_options(int what, CVarRef value /* = null_variant */) {
  if (what == k_ASSERT_ACTIVE) {
    int oldValue = s_option_data->assertActive;
    if (!value.isNull()) s_option_data->assertActive = value.toInt64();
    return oldValue;
  } else if (what == k_ASSERT_WARNING) {
    int oldValue = s_option_data->assertWarning;
    if (!value.isNull()) s_option_data->assertWarning = value.toInt64();
    return oldValue;
  } else if (what == k_ASSERT_BAIL) {
    int oldValue = s_option_data->assertBail;
    if (!value.isNull()) s_option_data->assertBail = value.toInt64();
    return oldValue;
  } else if (what == k_ASSERT_CALLBACK) {
    Variant oldValue = s_option_data->assertCallback;
    if (!value.isNull()) s_option_data->assertCallback = value;
    return oldValue;
  } else {
    throw InvalidArgumentException(0, "assert option %d is not supported",
                                   what);
  }
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
    Logger::Warning("Assertion failed");
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
  return
    !strcasecmp(name, "phpmcc") ||
    !strcasecmp(name, "bcmath") ||
    !strcasecmp(name, "spl") ||
    !strcasecmp(name, "curl") ||
    !strcasecmp(name, "simplexml") ||
    !strcasecmp(name, "mysql");
}

Array f_get_loaded_extensions(bool zend_extensions /* = false */) {
  throw NotSupportedException(__func__, "extensions are built differently");
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
    throw NotSupportedException(__func__, "constant categization not "
                                "supported");
  }
  return ClassInfo::GetConstants();
}

String f_get_include_path() {
  return "";
}

void f_restore_include_path() {
}

String f_set_include_path(CStrRef new_include_path) {
  return "";
}

Array f_get_included_files() {
  return Array::Create();
}

int f_get_magic_quotes_gpc() {
  return 0;
}

int f_get_magic_quotes_runtime() {
  throw NotSupportedException(__func__, "not using magic quotes");
}

Array f_get_required_files() {
  throw NotSupportedException(__func__, "requires PHP source code");
}

Variant f_getenv(CStrRef varname) {
  char *value = getenv((const char *)varname);
  if (value) {
    return String(value, CopyString);
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

Array f_getopt(CStrRef options, CVarRef longopts /* = null_variant */) {
  throw NotSupportedException(__func__, "global configurations not used");
}

#define PHP_RUSAGE_PARA(a) NEW(ArrayElement)(#a, (int64)usg.a)
Array f_getrusage(int who /* = 0 */) {
  struct rusage usg;
  memset(&usg, 0, sizeof(struct rusage));

  if (getrusage(who == 1 ? RUSAGE_CHILDREN : RUSAGE_SELF, &usg) == -1) {
    throw SystemCallFailure("getrusage");
  }

  return Array(PHP_RUSAGE_PARA(ru_oublock),
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
               PHP_RUSAGE_PARA(ru_utime.tv_usec),
               PHP_RUSAGE_PARA(ru_utime.tv_sec),
               PHP_RUSAGE_PARA(ru_stime.tv_usec),
               PHP_RUSAGE_PARA(ru_stime.tv_sec),
               NULL);
}

bool f_clock_getres(int clk_id, Variant sec, Variant nsec) {
  struct timespec ts;
  int ret = clock_getres(clk_id, &ts);
  sec = (int64)ts.tv_sec;
  nsec = (int64)ts.tv_nsec;
  return ret == 0;
}

bool f_clock_gettime(int clk_id, Variant sec, Variant nsec) {
  struct timespec ts;
  int ret = clock_gettime(clk_id, &ts);
  sec = (int64)ts.tv_sec;
  nsec = (int64)ts.tv_nsec;
  return ret == 0;
}

bool f_clock_settime(int clk_id, int64 sec, int64 nsec) {
  struct timespec ts;
  ts.tv_sec = sec;
  ts.tv_nsec = nsec;
  int ret = clock_settime(clk_id, &ts);
  return ret == 0;
}

String f_ini_alter(CStrRef varname, CStrRef newvalue) {
  throw NotSupportedException(__func__, "not using ini");
}

Array f_ini_get_all(CStrRef extension /* = null_string */) {
  throw NotSupportedException(__func__, "not using ini");
}

String f_ini_get(CStrRef varname) {
  if (varname == "error_reporting") {
    return g_context->getErrorReportingLevel();
  }
  if (varname == "memory_limit") {
    return (int64)RuntimeOption::RequestMemoryMaxBytes;
  }
  if (varname == "max_execution_time") {
    return (int64)RuntimeOption::RequestTimeoutSeconds;
  }
  return "";
}

void f_ini_restore(CStrRef varname) {
}

String f_ini_set(CStrRef varname, CStrRef newvalue) {
  return "";
}

int64 f_memory_get_peak_usage(bool real_usage /* = false */) {
  if (RuntimeOption::EnableMemoryManager) {
    MemoryManager *mm = MemoryManager::TheMemoryManager().get();
    const MemoryUsageStats &stats = mm->getStats();
    return real_usage ? stats.peakUsage : stats.peakAlloc;
  }
  return (int64)Process::GetProcessRSS(Process::GetProcessId()) * 1024 * 1024;
}

int64 f_memory_get_usage(bool real_usage /* = false */) {
  if (RuntimeOption::EnableMemoryManager) {
    MemoryManager *mm = MemoryManager::TheMemoryManager().get();
    const MemoryUsageStats &stats = mm->getStats();
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
  throw NotSupportedException(__func__, "setting environment variables is not a good coding practice in multithreading environment, and we are not supporting it until we find legitimate use cases, then maybe we will have dedicated functions to do them in thread-safe manner");
}

bool f_set_magic_quotes_runtime(bool new_setting) {
  if (new_setting) {
    throw NotSupportedException(__func__, "not using magic quotes");
  }
  return true;
}

void f_set_time_limit(int seconds) {
  TimeoutThread::DeferTimeout(seconds);
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
