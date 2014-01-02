/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include <pcre.h>
#include <onigposix.h>
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/thread-init-fini.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/ext/ext_string.h"
#include <tbb/concurrent_hash_map.h>

#define PREG_PATTERN_ORDER          1
#define PREG_SET_ORDER              2
#define PREG_OFFSET_CAPTURE         (1<<8)

#define PREG_SPLIT_NO_EMPTY         (1<<0)
#define PREG_SPLIT_DELIM_CAPTURE    (1<<1)
#define PREG_SPLIT_OFFSET_CAPTURE   (1<<2)

#define PREG_REPLACE_EVAL           (1<<0)

#define PREG_GREP_INVERT            (1<<0)

#define PCRE_CACHE_SIZE 4096

/* Only defined in pcre >= 8.32 */
#ifndef PCRE_STUDY_JIT_COMPILE
# define PCRE_STUDY_JIT_COMPILE 0
#endif

enum {
  PHP_PCRE_NO_ERROR = 0,
  PHP_PCRE_INTERNAL_ERROR,
  PHP_PCRE_BACKTRACK_LIMIT_ERROR,
  PHP_PCRE_RECURSION_LIMIT_ERROR,
  PHP_PCRE_BAD_UTF8_ERROR,
  PHP_PCRE_BAD_UTF8_OFFSET_ERROR
};

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// regex cache and helpers

class pcre_cache_entry {
  pcre_cache_entry(const pcre_cache_entry&);
  pcre_cache_entry& operator=(const pcre_cache_entry&);

public:
  pcre_cache_entry() {}
  ~pcre_cache_entry() {
    if (extra) free(extra); // we don't have pcre_free_study yet
    pcre_free(re);
  }

  pcre *re;
  pcre_extra *extra; // Holds results of studying
  int preg_options;
  int compile_options;
};

struct ahm_string_data_same {
  bool operator()(const StringData* s1, const StringData* s2) {
    // ahm uses -1, -2, -3 as magic values
    return int64_t(s1) > 0 && s1->same(s2);
  }
};
typedef folly::AtomicHashArray<const StringData*, const pcre_cache_entry*,
                         string_data_hash, ahm_string_data_same> PCREStringMap;
typedef std::pair<const StringData*, const pcre_cache_entry*> PCREEntry;

static PCREStringMap* s_pcreCacheMap;

void pcre_init() {
  if (!s_pcreCacheMap) {
    PCREStringMap::Config config;
    config.maxLoadFactor = 0.5;
    s_pcreCacheMap = PCREStringMap::create(
                       RuntimeOption::EvalPCRETableSize, config).release();
  }
}

void pcre_reinit() {
  PCREStringMap::Config config;
  config.maxLoadFactor = 0.5;
  PCREStringMap* newMap = PCREStringMap::create(
                     RuntimeOption::EvalPCRETableSize, config).release();
  if (s_pcreCacheMap) {
    PCREStringMap::iterator it;
    for (it = s_pcreCacheMap->begin(); it != s_pcreCacheMap->end(); it++) {
      // there should not be a lot of entries created before runtime
      // options were parsed.
      delete(it->second);
    }
    PCREStringMap::destroy(s_pcreCacheMap);
  }
  s_pcreCacheMap = newMap;
}

static const pcre_cache_entry* lookup_cached_pcre(const String& regex) {
  assert(s_pcreCacheMap);
  PCREStringMap::iterator it;
  if ((it = s_pcreCacheMap->find(regex.get())) != s_pcreCacheMap->end()) {
    return it->second;
  }
  return 0;
}

static const pcre_cache_entry*
insert_cached_pcre(const String& regex, const pcre_cache_entry* ent) {
  assert(s_pcreCacheMap);
  auto pair = s_pcreCacheMap->insert(
    PCREEntry(makeStaticString(regex.get()), ent));
  if (!pair.second) {
    delete ent;
    if (s_pcreCacheMap->size() < RuntimeOption::EvalPCRETableSize) {
      return pair.first->second;
    }
    // if the AHA is too small, fail.
    raise_error("PCRE cache full");
  }
  return ent;
}

/*
 * When a cached compiled pcre doesn't have pcre_extra, we use this
 * one.
 *
 * FIXME: It's unclear why this needs to be thread-local data instead
 * of just existing on the stack during the calls to preg_ functions.
 */
static __thread pcre_extra t_extra_data;

// The last pcre error code is available for the whole thread.
static __thread int t_last_error_code;

namespace {

void preg_init_thread_locals() {
  IniSetting::Bind("pcre.backtrack_limit",
                   std::to_string(RuntimeOption::PregBacktraceLimit).c_str(),
                   ini_on_update_long, ini_get_long,
                   &g_context->m_preg_backtrace_limit);
  IniSetting::Bind("pcre.recursion_limit",
                   std::to_string(RuntimeOption::PregRecursionLimit).c_str(),
                   ini_on_update_long, ini_get_long,
                   &g_context->m_preg_recursion_limit);
}
InitFiniNode init(preg_init_thread_locals, InitFiniNode::When::ThreadInit);

template<bool useSmartFree = false>
struct FreeHelperImpl : private boost::noncopyable {
  explicit FreeHelperImpl(void* p) : p(p) {}
  ~FreeHelperImpl() {
    useSmartFree ? smart_free(p) : free(p);
  }

private:
  void* p;
};

typedef FreeHelperImpl<true> SmartFreeHelper;
}

static const pcre_cache_entry*
pcre_get_compiled_regex_cache(const String& regex) {
  /* Try to lookup the cached regex entry, and if successful, just pass
     back the compiled pattern, otherwise go on and compile it. */
  if (const pcre_cache_entry* pce = lookup_cached_pcre(regex)) {
    return pce;
  }

  /* Parse through the leading whitespace, and display a warning if we
     get to the end without encountering a delimiter. */
  const char *p = regex.data();
  while (isspace((int)*(unsigned char *)p)) p++;
  if (*p == 0) {
    raise_warning("Empty regular expression");
    return nullptr;
  }

  /* Get the delimiter and display a warning if it is alphanumeric
     or a backslash. */
  char delimiter = *p++;
  if (isalnum((int)*(unsigned char *)&delimiter) || delimiter == '\\') {
    raise_warning("Delimiter must not be alphanumeric or backslash");
    return nullptr;
  }

  char start_delimiter = delimiter;
  const char *pp = strchr("([{< )]}> )]}>", delimiter);
  if (pp) {
    delimiter = pp[5];
  }
  char end_delimiter = delimiter;

  if (start_delimiter == end_delimiter) {
    /* We need to iterate through the pattern, searching for the ending
     * delimiter, but skipping the backslashed delimiters. If the ending
     * delimiter is not found, display a warning. */
    pp = p;
    while (*pp != 0) {
      if (*pp == '\\' && pp[1] != 0) pp++;
      else if (*pp == delimiter)
        break;
      pp++;
    }
    if (*pp == 0) {
      raise_warning("No ending delimiter '%c' found: [%s]", delimiter,
                      regex.data());
      return nullptr;
    }
  } else {
    /* We iterate through the pattern, searching for the matching ending
     * delimiter. For each matching starting delimiter, we increment nesting
     * level, and decrement it for each matching ending delimiter. If we
     * reach the end of the pattern without matching, display a warning.
     */
    int brackets = 1; // brackets nesting level
    pp = p;
    while (*pp != 0) {
      if (*pp == '\\' && pp[1] != 0) pp++;
      else if (*pp == end_delimiter && --brackets <= 0)
        break;
      else if (*pp == start_delimiter)
        brackets++;
      pp++;
    }
    if (*pp == 0) {
      raise_warning("No ending matching delimiter '%c' found: [%s]",
                      end_delimiter, regex.data());
      return nullptr;
    }
  }

  /* Make a copy of the actual pattern. */
  String spattern(p, pp-p, CopyString);
  const char *pattern = spattern.data();

  /* Move on to the options */
  pp++;

  /* Parse through the options, setting appropriate flags.  Display
     a warning if we encounter an unknown modifier. */
  int coptions = 0;
  int poptions = 0;
  bool do_study = false;
  while (*pp != 0) {
    switch (*pp++) {
      /* Perl compatible options */
    case 'i':  coptions |= PCRE_CASELESS;       break;
    case 'm':  coptions |= PCRE_MULTILINE;      break;
    case 's':  coptions |= PCRE_DOTALL;         break;
    case 'x':  coptions |= PCRE_EXTENDED;       break;

      /* PCRE specific options */
    case 'A':  coptions |= PCRE_ANCHORED;       break;
    case 'D':  coptions |= PCRE_DOLLAR_ENDONLY; break;
    case 'S':  do_study = true;                 break;
    case 'U':  coptions |= PCRE_UNGREEDY;       break;
    case 'X':  coptions |= PCRE_EXTRA;          break;
    case 'u':  coptions |= PCRE_UTF8;           break;

      /* Custom preg options */
    case 'e':  poptions |= PREG_REPLACE_EVAL;   break;

    case ' ':
    case '\n':
      break;

    default:
      raise_warning("Unknown modifier '%c': [%s]", pp[-1], regex.data());
      return nullptr;
    }
  }

  /* We've reached a null byte, now check if we're actually at the end of the
     string.  If not this is a bad expression, and a potential security hole. */
  if (regex.length() != (pp - regex.data())) {
    raise_error("Error: Null byte found in pattern");
  }

  /* Compile pattern and display a warning if compilation failed. */
  const char  *error;
  int erroffset;
  pcre *re = pcre_compile(pattern, coptions, &error, &erroffset, 0);
  if (re == nullptr) {
    raise_warning("Compilation failed: %s at offset %d", error, erroffset);
    return nullptr;
  }
  // Careful: from here 're' needs to be freed if something throws.

  /* If study option was specified, study the pattern and
     store the result in extra for passing to pcre_exec. */
  pcre_extra *extra = nullptr;
  if (do_study || PCRE_STUDY_JIT_COMPILE) {
    int soptions = PCRE_STUDY_JIT_COMPILE;
    extra = pcre_study(re, soptions, &error);
    if (extra) {
      extra->flags |= PCRE_EXTRA_MATCH_LIMIT |
        PCRE_EXTRA_MATCH_LIMIT_RECURSION;
    }
    if (error != nullptr) {
      try {
        raise_warning("Error while studying pattern");
      } catch (...) {
        pcre_free(re);
        throw;
      }
    }
  }

  /* Store the compiled pattern and extra info in the cache. */
  pcre_cache_entry *new_entry = new pcre_cache_entry();
  new_entry->re = re;
  new_entry->extra = extra;
  new_entry->preg_options = poptions;
  new_entry->compile_options = coptions;
  return insert_cached_pcre(regex, new_entry);
}

static void set_extra_limits(pcre_extra*& extra) {
  if (extra == nullptr) {
    pcre_extra& extra_data = t_extra_data;
    extra_data.flags = PCRE_EXTRA_MATCH_LIMIT |
      PCRE_EXTRA_MATCH_LIMIT_RECURSION;
    extra = &extra_data;
  }
  extra->match_limit = g_context->m_preg_backtrace_limit;
  extra->match_limit_recursion = g_context->m_preg_recursion_limit;
}

static int *create_offset_array(const pcre_cache_entry *pce,
                                int &size_offsets) {
  pcre_extra *extra = pce->extra;
  set_extra_limits(extra);

  /* Calculate the size of the offsets array, and allocate memory for it. */
  int num_subpats; // Number of captured subpatterns
  int rc = pcre_fullinfo(pce->re, extra, PCRE_INFO_CAPTURECOUNT, &num_subpats);
  if (rc < 0) {
    raise_warning("Internal pcre_fullinfo() error %d", rc);
    return nullptr;
  }
  num_subpats++;
  size_offsets = num_subpats * 3;
  return (int *)smart_malloc(size_offsets * sizeof(int));
}

/*
 * Build a mapping from subpattern numbers to their names. We will always
 * allocate the table, even though there may be no named subpatterns. This
 * avoids somewhat more complicated logic in the inner loops.
 */
static char** make_subpats_table(int num_subpats, const pcre_cache_entry* pce) {
  pcre_extra* extra = pce->extra;
  char **subpat_names = (char **)smart_calloc(num_subpats, sizeof(char *));
  int name_cnt = 0, name_size, ni = 0;
  char *name_table;
  unsigned short name_idx;

  int rc = pcre_fullinfo(pce->re, extra, PCRE_INFO_NAMECOUNT, &name_cnt);
  if (rc < 0) {
    raise_warning("Internal pcre_fullinfo() error %d", rc);
    return nullptr;
  }
  if (name_cnt > 0) {
    int rc1, rc2;
    rc1 = pcre_fullinfo(pce->re, extra, PCRE_INFO_NAMETABLE, &name_table);
    rc2 = pcre_fullinfo(pce->re, extra, PCRE_INFO_NAMEENTRYSIZE, &name_size);
    rc = rc2 ? rc2 : rc1;
    if (rc < 0) {
      raise_warning("Internal pcre_fullinfo() error %d", rc);
      return nullptr;
    }

    while (ni++ < name_cnt) {
      name_idx = 0xff * (unsigned char)name_table[0] +
                 (unsigned char)name_table[1];
      subpat_names[name_idx] = name_table + 2;
      if (is_numeric_string(subpat_names[name_idx],
                            strlen(subpat_names[name_idx]),
                            nullptr, nullptr, 0) != KindOfNull) {
        raise_warning("Numeric named subpatterns are not allowed");
        return nullptr;
      }
      name_table += name_size;
    }
  }
  return subpat_names;
}

static pcre* pcre_get_compiled_regex(const String& regex, pcre_extra **extra,
                                     int *preg_options) {
  const pcre_cache_entry* pce = pcre_get_compiled_regex_cache(regex);
  if (extra) {
    *extra = pce ? pce->extra : nullptr;
  }
  if (preg_options) {
    *preg_options = pce ? pce->preg_options : 0;
  }
  return pce ? pce->re : nullptr;
}

static inline void add_offset_pair(Variant &result, const String& str,
                                   int offset, const char *name) {
  ArrayInit match_pair(2);
  match_pair.set(str);
  match_pair.set(offset);
  Variant match_pair_v = match_pair.toVariant();
  if (name) result.set(String(name), match_pair_v);
  result.append(match_pair_v);
}

static inline bool pcre_need_log_error(int pcre_code) {
  return RuntimeOption::EnablePregErrorLog &&
         (pcre_code == PCRE_ERROR_MATCHLIMIT ||
          pcre_code == PCRE_ERROR_RECURSIONLIMIT);
}

static void pcre_log_error(const char *func, int line, int pcre_code,
                           const char *pattern, int pattern_size,
                           const char *subject, int subject_size,
                           const char *repl, int repl_size,
                           int arg1 = 0, int arg2 = 0,
                           int arg3 = 0, int arg4 = 0) {
  const char *escapedPattern;
  const char *escapedSubject;
  const char *escapedRepl;
  string p(pattern, pattern_size);
  string s(subject, subject_size);
  string r(repl, repl_size);
  escapedPattern = Logger::EscapeString(p);
  escapedSubject = Logger::EscapeString(s);
  escapedRepl = Logger::EscapeString(r);
  const char *errString =
    (pcre_code == PCRE_ERROR_MATCHLIMIT) ? "PCRE_ERROR_MATCHLIMIT" :
    (pcre_code == PCRE_ERROR_RECURSIONLIMIT) ? "PCRE_ERROR_RECURSIONLIMIT" :
    "UNKNOWN";
  raise_debugging(
    "REGEXERR: %s/%d: err=%d(%s), pattern='%s', subject='%s', repl='%s', "
    "limits=(%ld, %ld), extra=(%d, %d, %d, %d)",
    func, line, pcre_code, errString,
    escapedPattern, escapedSubject, escapedRepl,
    g_context->m_preg_backtrace_limit, g_context->m_preg_recursion_limit,
    arg1, arg2, arg3, arg4);
  free((void *)escapedPattern);
  free((void *)escapedSubject);
  free((void *)escapedRepl);
}

static void pcre_handle_exec_error(int pcre_code) {
  int preg_code = 0;
  switch (pcre_code) {
  case PCRE_ERROR_MATCHLIMIT:
    preg_code = PHP_PCRE_BACKTRACK_LIMIT_ERROR;
    break;
  case PCRE_ERROR_RECURSIONLIMIT:
    preg_code = PHP_PCRE_RECURSION_LIMIT_ERROR;
    break;
  case PCRE_ERROR_BADUTF8:
    preg_code = PHP_PCRE_BAD_UTF8_ERROR;
    break;
  case PCRE_ERROR_BADUTF8_OFFSET:
    preg_code = PHP_PCRE_BAD_UTF8_OFFSET_ERROR;
    break;
  default:
    preg_code = PHP_PCRE_INTERNAL_ERROR;
    break;
  }
  t_last_error_code = preg_code;
}

///////////////////////////////////////////////////////////////////////////////

Variant preg_grep(const String& pattern, CArrRef input, int flags /* = 0 */) {
  const pcre_cache_entry* pce = pcre_get_compiled_regex_cache(pattern);
  if (pce == nullptr) {
    return false;
  }

  int size_offsets = 0;
  int *offsets = create_offset_array(pce, size_offsets);
  if (offsets == nullptr) {
    return false;
  }
  SmartFreeHelper freer(offsets);

  /* Initialize return array */
  Array ret = Array::Create();
  t_last_error_code = PHP_PCRE_NO_ERROR;

  /* Go through the input array */
  bool invert = (flags & PREG_GREP_INVERT);
  pcre_extra *extra = pce->extra;
  set_extra_limits(extra);

  for (ArrayIter iter(input); iter; ++iter) {
    String entry = iter.second().toString();

    /* Perform the match */
    int count = pcre_exec(pce->re, extra, entry.data(), entry.size(),
                          0, 0, offsets, size_offsets);

    /* Check for too many substrings condition. */
    if (count == 0) {
      raise_warning("Matched, but too many substrings");
      count = size_offsets / 3;
    } else if (count < 0 && count != PCRE_ERROR_NOMATCH) {
      if (pcre_need_log_error(count)) {
        pcre_log_error(__FUNCTION__, __LINE__, count,
                       pattern.data(), pattern.size(),
                       entry.data(), entry.size(),
                       "", 0,
                       flags);
      }
      pcre_handle_exec_error(count);
      break;
    }

    /* If the entry fits our requirements */
    if ((count > 0 && !invert) ||
        (count == PCRE_ERROR_NOMATCH && invert)) {

      /* Add to return array */
      ret.set(iter.first(), entry);
    }
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

static Variant preg_match_impl(const String& pattern, const String& subject,
                               Variant *subpats, int flags, int start_offset,
                               bool global) {
  const pcre_cache_entry* pce = pcre_get_compiled_regex_cache(pattern);
  if (pce == nullptr) {
    return false;
  }

  pcre_extra *extra = pce->extra;
  set_extra_limits(extra);
  if (subpats) {
    *subpats = Array::Create();
  }

  int subpats_order = global ? PREG_PATTERN_ORDER : 0;
  bool offset_capture = false;
  if (flags) {
    offset_capture = flags & PREG_OFFSET_CAPTURE;

    /*
     * subpats_order is pre-set to pattern mode so we change it only if
     * necessary.
     */
    if (flags & 0xff) {
      subpats_order = flags & 0xff;
    }
    if ((global && (subpats_order < PREG_PATTERN_ORDER ||
                    subpats_order > PREG_SET_ORDER)) ||
        (!global && subpats_order != 0)) {
      raise_warning("Invalid flags specified");
      return null_variant;
    }
  }

  /* Negative offset counts from the end of the string. */
  if (start_offset < 0) {
    start_offset = subject.size() + start_offset;
    if (start_offset < 0) {
      start_offset = 0;
    }
  }

  int size_offsets = 0;
  int *offsets = create_offset_array(pce, size_offsets);
  SmartFreeHelper offsetsFreer(offsets);
  int num_subpats = size_offsets / 3;
  if (offsets == nullptr) {
    return false;
  }

  /*
   * Build a mapping from subpattern numbers to their names. We will always
   * allocate the table, even though there may be no named subpatterns. This
   * avoids somewhat more complicated logic in the inner loops.
   */
  char** subpat_names = make_subpats_table(num_subpats, pce);
  SmartFreeHelper subpatFreer(subpat_names);
  if (subpat_names == nullptr) {
    return false;
  }

  /* Allocate match sets array and initialize the values. */
  Array match_sets; /* An array of sets of matches for each
                       subpattern after a global match */
  if (global && subpats_order == PREG_PATTERN_ORDER) {
    for (int i = 0; i < num_subpats; i++) {
      match_sets.set(i, Array::Create());
    }
  }

  int matched = 0;
  t_last_error_code = PHP_PCRE_NO_ERROR;

  Variant result_set; // Holds a set of subpatterns after a global match
  int g_notempty = 0; // If the match should not be empty
  const char **stringlist; // Holds list of subpatterns
  int i;
  do {
    /* Execute the regular expression. */
    int count = pcre_exec(pce->re, extra, subject.data(), subject.size(),
                          start_offset, g_notempty, offsets, size_offsets);

    /* Check for too many substrings condition. */
    if (count == 0) {
      raise_warning("Matched, but too many substrings");
      count = size_offsets / 3;
    }

    /* If something has matched */
    if (count > 0) {
      matched++;

      if (subpats) {
        // Try to get the list of substrings and display a warning if failed.
        if (pcre_get_substring_list(subject.data(), offsets, count,
                                    &stringlist) < 0) {
          raise_warning("Get subpatterns list failed");
          return false;
        }

        if (global) {  /* global pattern matching */
          if (subpats_order == PREG_PATTERN_ORDER) {
            /* For each subpattern, insert it into the appropriate array. */
            for (i = 0; i < count; i++) {
              if (offset_capture) {
                add_offset_pair(match_sets.lvalAt(i),
                                String(stringlist[i],
                                       offsets[(i<<1)+1] - offsets[i<<1],
                                       CopyString),
                                offsets[i<<1], nullptr);
              } else {
                match_sets.lvalAt(i).append
                  (String(stringlist[i],
                          offsets[(i<<1)+1] - offsets[i<<1], CopyString));
              }
            }
            /*
             * If the number of captured subpatterns on this run is
             * less than the total possible number, pad the result
             * arrays with empty strings.
             */
            if (count < num_subpats) {
              for (; i < num_subpats; i++) {
                match_sets.lvalAt(i).append("");
              }
            }
          } else {
            result_set = Array::Create();

            /* Add all the subpatterns to it */
            for (i = 0; i < count; i++) {
              if (offset_capture) {
                add_offset_pair(result_set,
                                String(stringlist[i],
                                       offsets[(i<<1)+1] - offsets[i<<1],
                                       CopyString),
                                offsets[i<<1], subpat_names[i]);
              } else {
                String value(stringlist[i], offsets[(i<<1)+1] - offsets[i<<1],
                             CopyString);
                if (subpat_names[i]) {
                  result_set.set(String(subpat_names[i]), value);
                }
                result_set.append(value);
              }
            }
            /* And add it to the output array */
            subpats->append(result_set);
          }
        } else {      /* single pattern matching */
          /* For each subpattern, insert it into the subpatterns array. */
          for (i = 0; i < count; i++) {
            if (offset_capture) {
              add_offset_pair(*subpats,
                              String(stringlist[i],
                                     offsets[(i<<1)+1] - offsets[i<<1],
                                     CopyString),
                              offsets[i<<1], subpat_names[i]);
            } else {
              String value(stringlist[i], offsets[(i<<1)+1] - offsets[i<<1],
                           CopyString);
              if (subpat_names[i]) {
                subpats->set(String(subpat_names[i]), value);
              }
              subpats->append(value);
            }
          }
        }
        pcre_free((void *) stringlist);
      }
    } else if (count == PCRE_ERROR_NOMATCH) {
      /* If we previously set PCRE_NOTEMPTY after a null match,
         this is not necessarily the end. We need to advance
         the start offset, and continue. Fudge the offset values
         to achieve this, unless we're already at the end of the string. */
      if (g_notempty && start_offset < subject.size()) {
        offsets[0] = start_offset;
        offsets[1] = start_offset + 1;
      } else
        break;
    } else {
      if (pcre_need_log_error(count)) {
        pcre_log_error(__FUNCTION__, __LINE__, count,
                       pattern.data(), pattern.size(),
                       subject.data(), subject.size(),
                       "", 0,
                       flags, start_offset, g_notempty, global);
      }
      pcre_handle_exec_error(count);
      break;
    }

    /* If we have matched an empty string, mimic what Perl's /g options does.
       This turns out to be rather cunning. First we set PCRE_NOTEMPTY and try
       the match again at the same point. If this fails (picked up above) we
       advance to the next character. */
    g_notempty = (offsets[1] == offsets[0])? PCRE_NOTEMPTY | PCRE_ANCHORED : 0;

    /* Advance to the position right after the last full match */
    start_offset = offsets[1];
  } while (global);

  /* Add the match sets to the output array and clean up */
  if (subpats && global && subpats_order == PREG_PATTERN_ORDER) {
    for (i = 0; i < num_subpats; i++) {
      if (subpat_names[i]) {
        subpats->set(String(subpat_names[i]), match_sets[i]);
      }
      subpats->append(match_sets[i]);
    }
  }

  return matched;
}

Variant preg_match(const String& pattern, const String& subject,
                   Variant &matches, int flags /* = 0 */,
                   int offset /* = 0 */) {
  return preg_match_impl(pattern, subject, &matches, flags, offset, false);
}
Variant preg_match(const String& pattern, const String& subject,
                   int flags /* = 0 */, int offset /* = 0 */) {
  return preg_match_impl(pattern, subject, nullptr, flags, offset, false);
}

Variant preg_match_all(const String& pattern, const String& subject,
                       Variant &matches,
                       int flags /* = 0 */, int offset /* = 0 */) {
  return preg_match_impl(pattern, subject, &matches, flags, offset, true);
}
Variant preg_match_all(const String& pattern, const String& subject,
                       int flags /* = 0 */, int offset /* = 0 */) {
  return preg_match_impl(pattern, subject, nullptr, flags, offset, true);
}

///////////////////////////////////////////////////////////////////////////////

static String preg_do_repl_func(CVarRef function, const String& subject,
                                int* offsets, char** subpat_names, int count) {
  Array subpats = Array::Create();
  for (int i = 0; i < count; i++) {
    auto off1 = offsets[i<<1];
    auto off2 = offsets[(i<<1)+1];
    auto sub = subject.substr(off1, off2 - off1);

    if (subpat_names[i]) {
      subpats.set(String(subpat_names[i]), sub);
    }
    subpats.append(sub);
  }

  Array args;
  args.set(0, subpats);
  return vm_call_user_func(function, args);
}

static bool preg_get_backref(const char **str, int *backref) {
  char in_brace = 0;
  const char *walk = *str;

  if (walk[1] == 0) {
    return false;
  }

  if (*walk == '$' && walk[1] == '{') {
    in_brace = 1;
    walk++;
  }
  walk++;

  if (*walk >= '0' && *walk <= '9') {
    *backref = *walk - '0';
    walk++;
  } else {
    return false;
  }

  if (*walk && *walk >= '0' && *walk <= '9') {
    *backref = *backref * 10 + *walk - '0';
    walk++;
  }

  if (in_brace) {
    if (*walk == 0 || *walk != '}') {
      return false;
    }
    walk++;
  }

  *str = walk;
  return true;
}

static Variant php_pcre_replace(const String& pattern, const String& subject,
                                CVarRef replace_var, bool callable,
                                int limit, int *replace_count) {
  const pcre_cache_entry* pce = pcre_get_compiled_regex_cache(pattern);
  if (pce == nullptr) {
    return false;
  }
  bool eval = pce->preg_options & PREG_REPLACE_EVAL;
  if (eval) {
    if (RuntimeOption::RepoAuthoritative) {
      throw Exception(
        "You can't use eval in RepoAuthoritative mode. It breaks all sorts of "
        "assumptions we use for speed. Switch to using preg_replace_callback()."
      );
    }
    if (callable) {
      raise_warning(
        "Modifier /e cannot be used with replacement callback."
      );
      return null_string;
    }
    raise_notice(
      "Deprecated: preg_replace(): The /e modifier is deprecated, use "
      "preg_replace_callback instead"
    );
  }

  int size_offsets;
  int *offsets = create_offset_array(pce, size_offsets);
  SmartFreeHelper offsetsFreer(offsets);
  if (offsets == nullptr) {
    return false;
  }

  int num_subpats = size_offsets / 3;
  char** subpat_names = make_subpats_table(num_subpats, pce);
  SmartFreeHelper subpatNamesFreer(subpat_names);
  if (subpat_names == nullptr) {
    return false;
  }

  const char *replace = nullptr;
  const char *replace_end = nullptr;
  int replace_len = 0;
  String replace_val;

  if (!callable) {
    replace_val = replace_var.toString();
    replace = replace_val.data();
    replace_len = replace_val.size();
    replace_end = replace + replace_len;
  }

  int alloc_len = 2 * subject.size() + 1;
  char *result = (char *)malloc(alloc_len);

  try {

    /* Initialize */
    const char *match = nullptr;
    int start_offset = 0;
    t_last_error_code = PHP_PCRE_NO_ERROR;
    pcre_extra *extra = pce->extra;
    set_extra_limits(extra);

    int result_len = 0;
    int new_len;        // Length of needed storage
    const char *walk;   // Used to walk the replacement string
    char walk_last;     // Last walked character
    char *walkbuf;      // Location of current replacement in the result
    int match_len;      // Length of the current match
    int backref;        // Backreference number
    int g_notempty = 0; // If the match should not be empty
    while (1) {
      /* Execute the regular expression. */
      int count = pcre_exec(pce->re, extra, subject.data(), subject.size(),
                            start_offset, g_notempty, offsets, size_offsets);

      /* Check for too many substrings condition. */
      if (count == 0) {
        raise_warning("Matched, but too many substrings");
        count = size_offsets / 3;
      }

      const char *piece = subject.data() + start_offset;
      if (count > 0 && (limit == -1 || limit > 0)) {
        if (replace_count) {
          ++*replace_count;
        }
        /* Set the match location in subject */
        match = subject.data() + offsets[0];
        new_len = result_len + offsets[0] - start_offset; //part before the match

        /* If evaluating, do it and add the return string's length */
        String eval_result;
        if (callable) {
          /* Use custom function to get replacement string and its length. */
          eval_result = preg_do_repl_func(replace_var, subject, offsets,
                                          subpat_names, count);
          new_len += eval_result.size();
        } else { /* do regular substitution */
          walk = replace;
          walk_last = 0;
          while (walk < replace_end) {
            if ('\\' == *walk || '$' == *walk) {
              if (walk_last == '\\') {
                walk++;
                walk_last = 0;
                continue;
              }
              if (preg_get_backref(&walk, &backref)) {
                if (backref < count) {
                  match_len = offsets[(backref<<1)+1] - offsets[backref<<1];
                  if (eval) {
                    String esc_match = f_addslashes(
                      String(
                        subject.data() + offsets[backref<<1],
                        match_len,
                        CopyString
                      )
                    );
                    match_len = esc_match.length();
                  }
                  new_len += match_len;
                }
                continue;
              }
            }
            new_len++;
            walk++;
            walk_last = walk[-1];
          }
        }

        if (new_len + 1 > alloc_len) {
          alloc_len = 1 + alloc_len + 2 * new_len;
          result = (char *)realloc(result, alloc_len);
        }
        /* copy the part of the string before the match */
        memcpy(&result[result_len], piece, match-piece);
        result_len += match-piece;

        /* copy replacement and backrefs */
        walkbuf = result + result_len;

        /* If evaluating or using custom function, copy result to the buffer
         * and clean up. */
        if (callable) {
          memcpy(walkbuf, eval_result.data(), eval_result.size());
          result_len += eval_result.size();
        } else { /* do regular backreference copying */
          walk = replace;
          walk_last = 0;
          Array params;
          const char* lastStart = nullptr;
          while (walk < replace_end) {
            bool handleQuote = eval && '"' == *walk && walk_last != '\\';
            if (handleQuote && lastStart != nullptr) {
              String str(lastStart, walkbuf - lastStart, CopyString);
              params.append(str);
              lastStart = nullptr;
              handleQuote = false;
            }
            if ('\\' == *walk || '$' == *walk) {
              if (walk_last == '\\') {
                *(walkbuf-1) = *walk++;
                walk_last = 0;
                continue;
              }
              if (preg_get_backref(&walk, &backref)) {
                if (backref < count) {
                  match_len = offsets[(backref<<1)+1] - offsets[backref<<1];
                  if (eval) {
                    String esc_match = f_addslashes(
                      String(
                        subject.data() + offsets[backref<<1],
                        match_len,
                        CopyString
                      )
                    );
                    match_len = esc_match.length();
                    memcpy(walkbuf, esc_match.data(), match_len);
                  } else {
                    memcpy(
                      walkbuf,
                      subject.data() + offsets[backref<<1],
                      match_len
                    );
                  }
                  walkbuf += match_len;
                }
                continue;
              }
            }
            *walkbuf++ = *walk++;
            walk_last = walk[-1];
            if (handleQuote && lastStart == nullptr) {
              lastStart = walkbuf;
            }
          }
          *walkbuf = '\0';
          if (eval) {
            JIT::VMRegAnchor _;
            String prefixedCode = concat(concat(
                "<?php return ", result + result_len), ";");
            Unit* unit = g_vmContext->compileEvalString(prefixedCode.get());
            Variant v;
            Func* func = unit->getMain();
            g_vmContext->invokeFunc(v.asTypedValue(), func, init_null_variant,
                                    g_vmContext->getThis(),
                                    g_vmContext->getContextClass(), nullptr,
                                    nullptr,
                                    VMExecutionContext::InvokePseudoMain);
            eval_result = v;

            memcpy(result + result_len, eval_result.data(), eval_result.size());
            result_len += eval_result.size();
          } else {
            // increment the result length by how much we've added to the
            // string
            result_len += walkbuf - (result + result_len);
          }
        }

        if (limit != -1) {
          limit--;
        }

      } else if (count == PCRE_ERROR_NOMATCH || limit == 0) {
        /* If we previously set PCRE_NOTEMPTY after a null match,
           this is not necessarily the end. We need to advance
           the start offset, and continue. Fudge the offset values
           to achieve this, unless we're already at the end of the string. */
        if (g_notempty != 0 && start_offset < subject.size()) {
          offsets[0] = start_offset;
          offsets[1] = start_offset + 1;
          memcpy(&result[result_len], piece, 1);
          (result_len)++;
        } else {
          new_len = result_len + subject.size() - start_offset;
          if (new_len + 1 > alloc_len) {
            alloc_len = new_len + 1; /* now we know exactly how long it is */
            result = (char *)realloc(result, alloc_len);
          }
          /* stick that last bit of string on our output */
          memcpy(&result[result_len], piece, subject.size() - start_offset);
          result_len += subject.size() - start_offset;
          result[result_len] = '\0';
          break;
        }
      } else {
        if (pcre_need_log_error(count)) {
          const char *s;
          int size;
          String stemp;
          if (callable) {
            if (replace_var.isObject()) {
              stemp =
                replace_var.objectForCall()->o_getClassName() + "::__invoke";
            } else {
              stemp = replace_var.toString();
            }
            s = stemp.data();
            size = stemp.size();
          } else {
            s = replace_val.data();
            size = replace_val.size();
          }
          pcre_log_error(__FUNCTION__, __LINE__, count,
                         pattern.data(), pattern.size(),
                         subject.data(), subject.size(),
                         s, size,
                         callable, limit, start_offset, g_notempty);
        }
        pcre_handle_exec_error(count);
        free(result);
        result = nullptr;
        break;
      }

      /* If we have matched an empty string, mimic what Perl's /g options does.
         This turns out to be rather cunning. First we set PCRE_NOTEMPTY and try
         the match again at the same point. If this fails (picked up above) we
         advance to the next character. */
      g_notempty = (offsets[1] == offsets[0])? PCRE_NOTEMPTY | PCRE_ANCHORED : 0;

      /* Advance to the next piece. */
      start_offset = offsets[1];
    }

    if (result) {
      return String(result, result_len, AttachString);
    }
    return String();
  } catch (...) {
    free(result);
    throw;
  }
}

static Variant php_replace_in_subject(CVarRef regex, CVarRef replace,
                                      String subject, int limit, bool callable,
                                      int *replace_count) {
  if (!regex.is(KindOfArray)) {
    Variant ret = php_pcre_replace(regex.toString(), subject, replace,
                                   callable, limit, replace_count);

    if (ret.isBoolean()) {
      assert(!ret.toBoolean());
      return null_variant;
    }

    return ret;
  }

  if (callable || !replace.is(KindOfArray)) {
    Array arr = regex.toArray();
    for (ArrayIter iterRegex(arr); iterRegex; ++iterRegex) {
      String regex_entry = iterRegex.second().toString();
      Variant ret = php_pcre_replace(regex_entry, subject, replace,
                                     callable, limit, replace_count);
      if (ret.isBoolean()) {
        assert(!ret.toBoolean());
        return null_variant;
      }
      if (!ret.isString()) {
        return ret;
      }
      subject = ret.asStrRef();
      if (subject.isNull()) {
        return subject;
      }
    }
    return subject;
  }

  Array arrReplace = replace.toArray();
  Array arrRegex = regex.toArray();
  ArrayIter iterReplace(arrReplace);
  for (ArrayIter iterRegex(arrRegex); iterRegex; ++iterRegex) {
    String regex_entry = iterRegex.second().toString();
    Variant replace_value;
    if (iterReplace) {
      replace_value = iterReplace.second();
      ++iterReplace;
    }

    Variant ret = php_pcre_replace(regex_entry, subject, replace_value,
                                   callable, limit, replace_count);

    if (ret.isBoolean()) {
      assert(!ret.toBoolean());
      return null_variant;
    }
    if (!ret.isString()) {
      return ret;
    }
    subject = ret.asStrRef();
    if (subject.isNull()) {
      return subject;
    }
  }
  return subject;
}

Variant preg_replace_impl(CVarRef pattern, CVarRef replacement,
                          CVarRef subject, int limit, Variant &count,
                          bool is_callable) {
  if (!is_callable &&
      replacement.is(KindOfArray) && !pattern.is(KindOfArray)) {
    raise_warning("Parameter mismatch, pattern is a string while "
                    "replacement is an array");
    return false;
  }

  int replace_count = 0;
  if (!subject.is(KindOfArray)) {
    Variant ret = php_replace_in_subject(pattern, replacement,
                                         subject.toString(),
                                         limit, is_callable, &replace_count);

    if (ret.isString()) {
      count = replace_count;
      return ret.asStrRef();
    }

    return ret;
  }

  Array return_value = Array::Create();
  Array arrSubject = subject.toArray();
  for (ArrayIter iter(arrSubject); iter; ++iter) {
    String subject_entry = iter.second().toString();
    Variant ret = php_replace_in_subject(pattern, replacement, subject_entry,
                                         limit, is_callable, &replace_count);

    if (ret.isString() && !ret.isNull()) {
      return_value.set(iter.first(), ret.asStrRef());
    }
  }
  count = replace_count;
  return return_value;
}

int preg_replace(Variant &result, CVarRef pattern, CVarRef replacement,
                 CVarRef subject, int limit /* = -1 */) {
  Variant count;
  result = preg_replace_impl(pattern, replacement, subject, limit, count, false);
  return count.toInt32();
}

int preg_replace_callback(Variant &result, CVarRef pattern, CVarRef callback,
                          CVarRef subject, int limit /* = -1 */) {
  Variant count;
  result = preg_replace_impl(pattern, callback, subject, limit, count, true);
  return count.toInt32();
}

///////////////////////////////////////////////////////////////////////////////

Variant preg_split(CVarRef pattern, CVarRef subject, int limit /* = -1 */,
                   int flags /* = 0 */) {
  const pcre_cache_entry* pce = pcre_get_compiled_regex_cache(
    pattern.toString());
  if (pce == nullptr) {
    return false;
  }

  int no_empty = flags & PREG_SPLIT_NO_EMPTY;
  bool delim_capture = flags & PREG_SPLIT_DELIM_CAPTURE;
  bool offset_capture = flags & PREG_SPLIT_OFFSET_CAPTURE;

  if (limit == 0) {
    limit = -1;
  }

  int size_offsets = 0;
  int *offsets = create_offset_array(pce, size_offsets);
  SmartFreeHelper offsetsFreer(offsets);
  if (offsets == nullptr) {
    return false;
  }

  String ssubject = subject.toString();

  /* Start at the beginning of the string */
  int start_offset = 0;
  int next_offset = 0;
  const char *last_match = ssubject.data();
  t_last_error_code = PHP_PCRE_NO_ERROR;
  pcre_extra *extra = pce->extra;

  // Get next piece if no limit or limit not yet reached and something matched
  Variant return_value = Array::Create();
  int g_notempty = 0;   /* If the match should not be empty */
  int utf8_check = 0;
  pcre *re_bump = nullptr; /* Regex instance for empty matches */
  pcre_extra *extra_bump = nullptr; /* Almost dummy */
  while ((limit == -1 || limit > 1)) {
    int count = pcre_exec(pce->re, extra, ssubject.data(), ssubject.size(),
                          start_offset, g_notempty | utf8_check,
                          offsets, size_offsets);

    /* Check for too many substrings condition. */
    if (count == 0) {
      raise_warning("Matched, but too many substrings");
      count = size_offsets / 3;
    }

    /* If something matched */
    if (count > 0) {
      /* Subsequent calls to pcre_exec don't need to bother with the
       * utf8 validity check: if the subject isn't valid, the first
       * call to pcre_exec will have failed, and as long as we only
       * set start_offset to known character boundaries we won't
       * supply an invalid offset. */
      utf8_check = PCRE_NO_UTF8_CHECK;

      if (!no_empty || ssubject.data() + offsets[0] != last_match) {
        if (offset_capture) {
          /* Add (match, offset) pair to the return value */
          add_offset_pair(return_value,
                          String(last_match,
                                 ssubject.data() + offsets[0] - last_match,
                                 CopyString),
                          next_offset, nullptr);
        } else {
          /* Add the piece to the return value */
          return_value.append(String(last_match,
                                     ssubject.data() + offsets[0] - last_match,
                                     CopyString));
        }

        /* One less left to do */
        if (limit != -1)
          limit--;
      }

      last_match = ssubject.data() + offsets[1];
      next_offset = offsets[1];

      if (delim_capture) {
        int i, match_len;
        for (i = 1; i < count; i++) {
          match_len = offsets[(i<<1)+1] - offsets[i<<1];
          /* If we have matched a delimiter */
          if (!no_empty || match_len > 0) {
            if (offset_capture) {
              add_offset_pair(return_value,
                              String(ssubject.data() + offsets[i<<1],
                                     match_len, CopyString),
                              offsets[i<<1], nullptr);
            } else {
              return_value.append(ssubject.substr(offsets[i<<1], match_len));
            }
          }
        }
      }
    } else if (count == PCRE_ERROR_NOMATCH) {
      /* If we previously set PCRE_NOTEMPTY after a null match,
         this is not necessarily the end. We need to advance
         the start offset, and continue. Fudge the offset values
         to achieve this, unless we're already at the end of the string. */
      if (g_notempty != 0 && start_offset < ssubject.size()) {
        if (pce->compile_options & PCRE_UTF8) {
          if (re_bump == nullptr) {
            int dummy;
            if ((re_bump = pcre_get_compiled_regex("/./us", &extra_bump,
                                                   &dummy)) == nullptr) {
              return false;
            }
          }
          count = pcre_exec(re_bump, extra_bump, ssubject.data(),
                            ssubject.size(), start_offset,
                            0, offsets, size_offsets);
          if (count < 1) {
            raise_warning("Unknown error");
            offsets[0] = start_offset;
            offsets[1] = start_offset + 1;
            if (pcre_need_log_error(count)) {
              String spattern = pattern.toString();
              pcre_log_error(__FUNCTION__, __LINE__, count,
                             spattern.data(), spattern.size(),
                             ssubject.data(), ssubject.size(),
                             "", 0,
                             limit, flags, start_offset);
            }
          }
        } else {
          offsets[0] = start_offset;
          offsets[1] = start_offset + 1;
        }
      } else
        break;
    } else {
      if (pcre_need_log_error(count)) {
        String spattern = pattern.toString();
        pcre_log_error(__FUNCTION__, __LINE__, count,
                       spattern.data(), spattern.size(),
                       ssubject.data(), ssubject.size(),
                       "", 0,
                       limit, flags, start_offset, g_notempty);
      }
      pcre_handle_exec_error(count);
      break;
    }

    /* If we have matched an empty string, mimic what Perl's /g options does.
       This turns out to be rather cunning. First we set PCRE_NOTEMPTY and try
       the match again at the same point. If this fails (picked up above) we
       advance to the next character. */
    g_notempty = (offsets[1] == offsets[0])? PCRE_NOTEMPTY | PCRE_ANCHORED : 0;

    /* Advance to the position right after the last full match */
    start_offset = offsets[1];
  }

  start_offset = last_match - ssubject.data(); /* offset might have
                                                * been incremented,
                                                * but without further
                                                * successful matches */
  if (!no_empty || start_offset < ssubject.size()) {
    if (offset_capture) {
      /* Add the last (match, offset) pair to the return value */
      add_offset_pair(return_value,
                      ssubject.substr(start_offset),
                      start_offset, nullptr);
    } else {
      /* Add the last piece to the return value */
      return_value.append
        (String(last_match, ssubject.data() + ssubject.size() - last_match,
                CopyString));
    }
  }

  return return_value;
}

///////////////////////////////////////////////////////////////////////////////

String preg_quote(const String& str,
                  const String& delimiter /* = null_string */) {
  const char *in_str = str.data();
  const char *in_str_end = in_str + str.size();

  /* Nothing to do if we got an empty string */
  if (in_str == in_str_end) {
    return str;
  }

  char delim_char = 0;      /* Delimiter character to be quoted */
  bool quote_delim = false; /* Whether to quote additional delim char */
  if (!delimiter.empty()) {
    delim_char = delimiter.charAt(0);
    quote_delim = true;
  }

  /* Allocate enough memory so that even if each character
     is quoted, we won't run out of room */
  char *out_str = (char *)malloc(4 * str.size() + 1);

  /* Go through the string and quote necessary characters */
  const char *p;
  char *q;
  for (p = in_str, q = out_str; p != in_str_end; p++) {
    char c = *p;
    switch (c) {
    case '.': case '\\': case '+': case '*': case '?':
    case '[': case '^':  case ']': case '$': case '(':
    case ')': case '{':  case '}': case '=': case '!':
    case '>': case '<':  case '|': case ':':
      *q++ = '\\';
      *q++ = c;
      break;

    case '\0':
      *q++ = '\\';
      *q++ = '0';
      *q++ = '0';
      *q++ = '0';
      break;

    default:
      if (quote_delim && c == delim_char)
        *q++ = '\\';
      *q++ = c;
      break;
    }
  }
  *q = '\0';

  return String(out_str, q - out_str, AttachString);
}

int preg_last_error() {
  return t_last_error_code;
}

size_t preg_pcre_cache_size() {
  return (size_t)s_pcreCacheMap->size();
}

///////////////////////////////////////////////////////////////////////////////
// regexec

static void php_reg_eprint(int err, regex_t *re) {
  char *buf = nullptr, *message = nullptr;
  size_t len;
  size_t buf_len;

#ifdef REG_ITOA
  /* get the length of the message */
  buf_len = regerror(REG_ITOA | err, re, nullptr, 0);
  if (buf_len) {
    buf = (char *)smart_malloc(buf_len);
    if (!buf) return; /* fail silently */
    /* finally, get the error message */
    regerror(REG_ITOA | err, re, buf, buf_len);
  }
#else
  buf_len = 0;
#endif
  len = regerror(err, re, nullptr, 0);
  if (len) {
    message = (char *)smart_malloc(buf_len + len + 2);
    if (!message) {
      return; /* fail silently */
    }
    if (buf_len) {
      snprintf(message, buf_len, "%s: ", buf);
      buf_len += 1; /* so pointer math below works */
    }
    /* drop the message into place */
    regerror(err, re, message + buf_len, len);
    raise_warning("%s", message);
  }
  smart_free(buf);
  smart_free(message);
}

Variant php_split(const String& spliton, const String& str, int count,
                  bool icase) {
  const char *strp = str.data();
  const char *endp = strp + str.size();

  regex_t re;
  int copts = icase ? REG_ICASE : 0;
  int err = regcomp(&re, spliton.data(), REG_EXTENDED | copts);
  if (err) {
    php_reg_eprint(err, &re);
    return false;
  }

  Array return_value = Array::Create();
  regmatch_t subs[1];

  /* churn through str, generating array entries as we go */
  while ((count == -1 || count > 1) &&
         !(err = regexec(&re, strp, 1, subs, 0))) {
    if (subs[0].rm_so == 0 && subs[0].rm_eo) {
      /* match is at start of string, return empty string */
      return_value.append("");
      /* skip ahead the length of the regex match */
      strp += subs[0].rm_eo;
    } else if (subs[0].rm_so == 0 && subs[0].rm_eo == 0) {
      /* No more matches */
      regfree(&re);
      raise_warning("Invalid Regular Expression to split()");
      return false;
    } else {
      /* On a real match */

      /* make a copy of the substring */
      int size = subs[0].rm_so;

      /* add it to the array */
      return_value.append(String(strp, size, CopyString));

      /* point at our new starting point */
      strp = strp + subs[0].rm_eo;
    }

    /* if we're only looking for a certain number of points,
       stop looking once we hit it */
    if (count != -1) {
      count--;
    }
  }

  /* see if we encountered an error */
  if (err && err != REG_NOMATCH) {
    php_reg_eprint(err, &re);
    regfree(&re);
    return false;
  }

  /* otherwise we just have one last element to add to the array */
  int size = endp - strp;
  return_value.append(String(strp, size, CopyString));

  regfree(&re);
  return return_value;
}

///////////////////////////////////////////////////////////////////////////////
}
