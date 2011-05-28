/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <runtime/base/string_util.h>
#include <runtime/base/util/request_local.h>
#include <util/lock.h>
#include <util/logger.h>
#include <pcre.h>
#include <onigposix.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/zend/zend_functions.h>
#include <runtime/base/array/array_iterator.h>
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

enum {
  PHP_PCRE_NO_ERROR = 0,
  PHP_PCRE_INTERNAL_ERROR,
  PHP_PCRE_BACKTRACK_LIMIT_ERROR,
  PHP_PCRE_RECURSION_LIMIT_ERROR,
  PHP_PCRE_BAD_UTF8_ERROR,
  PHP_PCRE_BAD_UTF8_OFFSET_ERROR
};

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// regex cache and helpers

class pcre_cache_entry {
public:
  ~pcre_cache_entry() {
    free(re);
    if (extra) free(extra);
#if HAVE_SETLOCALE
    free(locale);
    if (tables) free(tables);
#endif
  }

  pcre *re;
  pcre_extra *extra; // Holds results of studying
  int preg_options;
#if HAVE_SETLOCALE
  char *locale;
  unsigned const char *tables;
#endif
  int compile_options;
};


// TODO LRU cache
class PCRECache {
public:
  ~PCRECache() { }

  void cleanup() {
    WriteLock l(m_lock);
    for (PCREStringMap::iterator it = m_cache.begin(); it != m_cache.end();
         ++it) {
      delete it->second;
      if (!it->first->isStatic()) {
        delete it->first;
      }
    }
  }

  pcre_cache_entry *find(CStrRef regex) {
    ReadLock l(m_lock);
    PCREStringMap::const_accessor acc;
    if (!m_cache.find(acc, regex.get())) {
      return NULL;
    }
    return acc->second;
  }

  void set(CStrRef regex, pcre_cache_entry *pce) {
    ReadLock l(m_lock);
    PCREStringMap::accessor acc;
    if (m_cache.find(acc, regex.get())) {
      delete acc->second;
    } else {
      m_cache.insert(acc, regex->copy(true));
    }
    acc->second = pce;
  }

private:
  typedef tbb::concurrent_hash_map <StringData *, pcre_cache_entry *,
                                    StringDataHashCompare> PCREStringMap;
  ReadWriteMutex m_lock;
  PCREStringMap m_cache;
};

static PCRECache s_pcre_cache;

class PCRELocal {
public:
  ~PCRELocal() { }
  int error_code;
  pcre_extra extra_data;
};
IMPLEMENT_THREAD_LOCAL_NO_CHECK(PCRELocal, s_pcre_local);

void preg_get_pcre_cache() {
  s_pcre_local.getCheck();
}

static pcre_cache_entry *pcre_get_compiled_regex_cache(CStrRef regex) {

  /* Try to lookup the cached regex entry, and if successful, just pass
     back the compiled pattern, otherwise go on and compile it. */
  pcre_cache_entry *pce = s_pcre_cache.find(regex);
  if (pce) {
    /**
     * We use a quick pcre_info() check to see whether cache is corrupted,
     * and if it is, we flush it and compile the pattern from scratch.
     */
    if (pcre_info(pce->re, NULL, NULL) == PCRE_ERROR_BADMAGIC) {
      s_pcre_cache.cleanup();
    } else {
#if HAVE_SETLOCALE
      if (!strcmp(pce->locale, locale)) {
#endif
        return pce;
#if HAVE_SETLOCALE
      }
#endif
    }
  }

  /* Parse through the leading whitespace, and display a warning if we
     get to the end without encountering a delimiter. */
  const char *p = regex.data();
  while (isspace((int)*(unsigned char *)p)) p++;
  if (*p == 0) {
    raise_warning("Empty regular expression");
    return NULL;
  }

  /* Get the delimiter and display a warning if it is alphanumeric
     or a backslash. */
  char delimiter = *p++;
  if (isalnum((int)*(unsigned char *)&delimiter) || delimiter == '\\') {
    raise_warning("Delimiter must not be alphanumeric or backslash");
    return NULL;
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
      return NULL;
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
      return NULL;
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
  int do_study = false;
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
      return NULL;
    }
  }

  unsigned const char *tables = NULL;
#if HAVE_SETLOCALE
  if (strcmp(locale, "C")) {
    tables = pcre_maketables();
  }
#endif

  /* Compile pattern and display a warning if compilation failed. */
  const char  *error;
  int erroffset;
  pcre *re = pcre_compile(pattern, coptions, &error, &erroffset, tables);
  if (re == NULL) {
    raise_warning("Compilation failed: %s at offset %d", error, erroffset);
    if (tables) {
      free((void*)tables);
    }
    return NULL;
  }

  /* If study option was specified, study the pattern and
     store the result in extra for passing to pcre_exec. */
  pcre_extra *extra = NULL;
  if (do_study) {
    int soptions = 0;
    extra = pcre_study(re, soptions, &error);
    if (extra) {
      extra->flags |= PCRE_EXTRA_MATCH_LIMIT |
        PCRE_EXTRA_MATCH_LIMIT_RECURSION;
    }
    if (error != NULL) {
      raise_warning("Error while studying pattern");
    }
  }

  /* Store the compiled pattern and extra info in the cache. */
  pcre_cache_entry *new_entry = new pcre_cache_entry();
  new_entry->re = re;
  new_entry->extra = extra;
  new_entry->preg_options = poptions;
  new_entry->compile_options = coptions;
#if HAVE_SETLOCALE
  char *locale = setlocale(LC_CTYPE, NULL);
  new_entry->locale = strdup(locale);
  new_entry->tables = tables;
#endif
  s_pcre_cache.set(regex, new_entry);
  return new_entry;
}

static void set_extra_limits(pcre_extra *&extra) {
  if (extra == NULL) {
    pcre_extra &extra_data = s_pcre_local->extra_data;
    extra_data.flags = PCRE_EXTRA_MATCH_LIMIT |
      PCRE_EXTRA_MATCH_LIMIT_RECURSION;
    extra = &extra_data;
  }
  extra->match_limit = RuntimeOption::PregBacktraceLimit;
  extra->match_limit_recursion = RuntimeOption::PregRecursionLimit;
}

static int *create_offset_array(pcre_cache_entry *pce, int &size_offsets) {
  pcre_extra *extra = pce->extra;
  set_extra_limits(extra);

  /* Calculate the size of the offsets array, and allocate memory for it. */
  int num_subpats; // Number of captured subpatterns
  int rc = pcre_fullinfo(pce->re, extra, PCRE_INFO_CAPTURECOUNT, &num_subpats);
  if (rc < 0) {
    raise_warning("Internal pcre_fullinfo() error %d", rc);
    return NULL;
  }
  num_subpats++;
  size_offsets = num_subpats * 3;
  return (int *)malloc(size_offsets * sizeof(int));
}

static pcre* pcre_get_compiled_regex(CStrRef regex, pcre_extra **extra,
                                     int *preg_options) {
  pcre_cache_entry *pce = pcre_get_compiled_regex_cache(regex);
  if (extra) {
    *extra = pce ? pce->extra : NULL;
  }
  if (preg_options) {
    *preg_options = pce ? pce->preg_options : 0;
  }
  return pce ? pce->re : NULL;
}

static inline void add_offset_pair(Variant &result, CStrRef str, int offset,
                                   const char *name) {
  Array match_pair;
  match_pair.append(str);
  match_pair.append(offset);

  if (name) {
    result.set(name, match_pair);
  }
  result.append(match_pair);
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
  raise_debugging(
    "REGEXERR: %s/%d: err=%d, pattern='%s', subject='%s', repl='%s', "
    "limits=(%d, %d), extra=(%d, %d, %d, %d)",
    func, line, pcre_code,
    escapedPattern, escapedSubject, escapedRepl,
    RuntimeOption::PregBacktraceLimit, RuntimeOption::PregRecursionLimit,
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
  s_pcre_local->error_code = preg_code;
}

///////////////////////////////////////////////////////////////////////////////

Variant preg_grep(CStrRef pattern, CArrRef input, int flags /* = 0 */) {
  pcre_cache_entry *pce = pcre_get_compiled_regex_cache(pattern);
  if (pce == NULL) {
    return false;
  }

  int size_offsets = 0;
  int *offsets = create_offset_array(pce, size_offsets);
  if (offsets == NULL) {
    return false;
  }

  /* Initialize return array */
  Array ret = Array::Create();
  s_pcre_local->error_code = PHP_PCRE_NO_ERROR;

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

  /* Clean up */
  free(offsets);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

static Variant preg_match_impl(CStrRef pattern, CStrRef subject,
                               Variant *subpats, int flags, int start_offset,
                               bool global) {
  pcre_cache_entry *pce = pcre_get_compiled_regex_cache(pattern);
  if (pce == NULL) {
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
      return false;
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
  int num_subpats = size_offsets / 3;
  if (offsets == NULL) {
    return false;
  }

  /*
   * Build a mapping from subpattern numbers to their names. We will always
   * allocate the table, even though there may be no named subpatterns. This
   * avoids somewhat more complicated logic in the inner loops.
   */
  char **subpat_names = (char **)malloc(num_subpats * sizeof(char *));
  memset(subpat_names, 0, sizeof(char *) * num_subpats);
  {
    int name_cnt = 0, name_size, ni = 0;
    char *name_table;
    unsigned short name_idx;

    int rc = pcre_fullinfo(pce->re, extra, PCRE_INFO_NAMECOUNT, &name_cnt);
    if (rc < 0) {
      raise_warning("Internal pcre_fullinfo() error %d", rc);
      free(offsets);
      free(subpat_names);
      return false;
    }
    if (name_cnt > 0) {
      int rc1, rc2;
      rc1 = pcre_fullinfo(pce->re, extra, PCRE_INFO_NAMETABLE, &name_table);
      rc2 = pcre_fullinfo(pce->re, extra, PCRE_INFO_NAMEENTRYSIZE, &name_size);
      rc = rc2 ? rc2 : rc1;
      if (rc < 0) {
        raise_warning("Internal pcre_fullinfo() error %d", rc);
        free(offsets);
        free(subpat_names);
        return false;
      }

      while (ni++ < name_cnt) {
        name_idx = 0xff * (unsigned char)name_table[0] +
                   (unsigned char)name_table[1];
        subpat_names[name_idx] = name_table + 2;
        if (is_numeric_string(subpat_names[name_idx],
                              strlen(subpat_names[name_idx]),
                              NULL, NULL, 0) != KindOfNull) {
          raise_warning("Numeric named subpatterns are not allowed");
          free(offsets);
          free(subpat_names);
          return false;
        }
        name_table += name_size;
      }
    }
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
  s_pcre_local->error_code = PHP_PCRE_NO_ERROR;

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

      if (!subpats) continue;

      // Try to get the list of substrings and display a warning if failed.
      if (pcre_get_substring_list(subject.data(), offsets, count,
                                  &stringlist) < 0) {
        free(offsets);
        free(subpat_names);
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
                              offsets[i<<1], NULL);
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
                result_set.set(subpat_names[i], value);
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
              subpats->set(subpat_names[i], value);
            }
            subpats->append(value);
          }
        }
      }

      pcre_free((void *) stringlist);
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
        subpats->set(subpat_names[i], match_sets[i]);
      }
      subpats->append(match_sets[i]);
    }
  }

  free(offsets);
  free(subpat_names);
  return matched;
}

Variant preg_match(CStrRef pattern, CStrRef subject,
                   Variant &matches, int flags /* = 0 */,
                   int offset /* = 0 */) {
  return preg_match_impl(pattern, subject, &matches, flags, offset, false);
}
Variant preg_match(CStrRef pattern, CStrRef subject, int flags /* = 0 */,
                   int offset /* = 0 */) {
  return preg_match_impl(pattern, subject, NULL, flags, offset, false);
}

Variant preg_match_all(CStrRef pattern, CStrRef subject, Variant &matches,
                       int flags /* = 0 */, int offset /* = 0 */) {
  return preg_match_impl(pattern, subject, &matches, flags, offset, true);
}
Variant preg_match_all(CStrRef pattern, CStrRef subject,
                       int flags /* = 0 */, int offset /* = 0 */) {
  return preg_match_impl(pattern, subject, NULL, flags, offset, true);
}

///////////////////////////////////////////////////////////////////////////////

static String preg_do_repl_func(CVarRef function, CStrRef subject,
                                int *offsets, int count) {
  Array subpats = Array::Create();
  for (int i = 0; i < count; i++) {
    subpats.append(subject.substr(offsets[i<<1],
                                  offsets[(i<<1)+1] - offsets[i<<1]));
  }

  Array args;
  args.set(0, subpats);
  return f_call_user_func_array(function, args);
}

static bool preg_get_backref(const char **str, int *backref) {
  register char in_brace = 0;
  register const char *walk = *str;

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

static String php_pcre_replace(CStrRef pattern, CStrRef subject,
                               CVarRef replace_var, bool callable,
                               int limit, int *replace_count) {
  pcre_cache_entry *pce = pcre_get_compiled_regex_cache(pattern);
  if (pce == NULL) {
    return false;
  }
  bool eval = false;
  if (pce->preg_options & PREG_REPLACE_EVAL) {
    if (callable)
      throw NotSupportedException("preg_replace",
                                  "Modifier /e cannot be used with replacement "
                                  "callback.");
    eval = true;
  }

  int size_offsets;
  int *offsets = create_offset_array(pce, size_offsets);
  if (offsets == NULL) {
    return false;
  }

  const char *replace = NULL;
  const char *replace_end = NULL;
  int replace_len = 0;
  String replace_val = replace_var.toString();
  String eval_fn;
  if (eval) {
    // Extract eval fn
    int pidx = replace_val.find('(');
    const char *rd = replace_val.data();
    int rs = replace_val.size();

    if (!(rs >= 5 && pidx >= 0 && rd[pidx+1] == '"' &&
          ((rd[rs-2] == '"' && rd[rs-1] == ')') ||
           (rd[rs-3] == '"' && rd[rs-2] == ')' && rd[rs-1] == ';')))) {
      throw NotSupportedException("preg_replace",
                                  "Modifier /e must be used with the form "
                                  "f(\"<replacement string>\") or "
                                  "f(\"<replacement string>\");");
    }
    eval_fn = replace_val.substr(0, pidx);
    replace_val = replace_val.substr(pidx+1, rs - (pidx+1) - 1);
  }
  if (!callable) {
    replace = replace_val.data();
    replace_len = replace_val.size();
    replace_end = replace + replace_len;
  }

  int alloc_len = 2 * subject.size() + 1;
  char *result = (char *)malloc(alloc_len);

  /* Initialize */
  const char *match = NULL;
  int start_offset = 0;
  s_pcre_local->error_code = PHP_PCRE_NO_ERROR;
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
        eval_result = preg_do_repl_func(replace_var, subject, offsets, count);
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
                new_len += offsets[(backref<<1)+1] - offsets[backref<<1];
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
        while (walk < replace_end) {
          if ('\\' == *walk || '$' == *walk) {
            if (walk_last == '\\') {
              *(walkbuf-1) = *walk++;
              walk_last = 0;
              continue;
            }
            if (preg_get_backref(&walk, &backref)) {
              if (backref < count) {
                match_len = offsets[(backref<<1)+1] - offsets[backref<<1];
                memcpy(walkbuf, subject.data() + offsets[backref<<1],
                       match_len);
                walkbuf += match_len;
              }
              continue;
            }
          }
          *walkbuf++ = *walk++;
          walk_last = walk[-1];
        }
        *walkbuf = '\0';
        if (eval) {
          String args(result + result_len, walkbuf - (result + result_len),
                     CopyString);
          Array params;
          bool slashed = false;
          int lastStart = -1;
          for (int i = 0; i < args.size(); i++) {
            char c = args.charAt(i);
            if (slashed) {
              slashed = false;
              continue;
            }
            switch (c) {
            case '"':
              if (lastStart >= 0) {
                params.append(args.substr(lastStart, i - lastStart));
                lastStart = -1;
              } else {
                lastStart = i + 1;
              }
              break;
            case '\\':
              slashed = true;
              break;
            }
          }

          eval_result = f_call_user_func_array(eval_fn, params);
          memcpy(result + result_len, eval_result.data(), eval_result.size());
          result_len += eval_result.size();
        } else {
          /* increment the result length by how much we've added to the string */
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
        pcre_log_error(__FUNCTION__, __LINE__, count,
                       pattern.data(), pattern.size(),
                       subject.data(), subject.size(),
                       replace_val.data(), replace_val.size(),
                       callable, limit, start_offset, g_notempty);
      }
      pcre_handle_exec_error(count);
      free(result);
      result = NULL;
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

  free(offsets);
  if (result) {
    return String(result, result_len, AttachString);
  }
  return String();
}

static String php_replace_in_subject(CVarRef regex, CVarRef replace,
                                     String subject, int limit, bool callable,
                                     int *replace_count) {
  if (!regex.is(KindOfArray)) {
    return php_pcre_replace(regex.toString(), subject, replace,
                            callable, limit, replace_count);
  }

  if (callable || !replace.is(KindOfArray)) {
    Array arr = regex.toArray();
    for (ArrayIter iterRegex(arr); iterRegex; ++iterRegex) {
      String regex_entry = iterRegex.second().toString();
      subject = php_pcre_replace(regex_entry, subject, replace,
                                 callable, limit, replace_count);
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

    subject = php_pcre_replace(regex_entry, subject, replace_value,
                               callable, limit, replace_count);
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
    String ret = php_replace_in_subject(pattern, replacement,
                                        subject.toString(),
                                        limit, is_callable, &replace_count);
    count = replace_count;
    return ret;
  }

  Array return_value;
  Array arrSubject = subject.toArray();
  for (ArrayIter iter(arrSubject); iter; ++iter) {
    String subject_entry = iter.second().toString();
    String result = php_replace_in_subject(pattern, replacement, subject_entry,
                                           limit, is_callable, &replace_count);
    if (!result.isNull()) {
      return_value.set(iter.first(), result);
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
  pcre_cache_entry *pce = pcre_get_compiled_regex_cache(pattern.toString());
  if (pce == NULL) {
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
  if (offsets == NULL) {
    return false;
  }

  String ssubject = subject.toString();

  /* Start at the beginning of the string */
  int start_offset = 0;
  int next_offset = 0;
  const char *last_match = ssubject.data();
  s_pcre_local->error_code = PHP_PCRE_NO_ERROR;
  pcre_extra *extra = pce->extra;

  // Get next piece if no limit or limit not yet reached and something matched
  Variant return_value = Array::Create();
  int g_notempty = 0;   /* If the match should not be empty */
  pcre *re_bump = NULL; /* Regex instance for empty matches */
  pcre_extra *extra_bump = NULL; /* Almost dummy */
  while ((limit == -1 || limit > 1)) {
    int count = pcre_exec(pce->re, extra, ssubject.data(), ssubject.size(),
                          start_offset, g_notempty, offsets, size_offsets);

    /* Check for too many substrings condition. */
    if (count == 0) {
      raise_warning("Matched, but too many substrings");
      count = size_offsets / 3;
    }

    /* If something matched */
    if (count > 0) {

      if (!no_empty || ssubject.data() + offsets[0] != last_match) {
        if (offset_capture) {
          /* Add (match, offset) pair to the return value */
          add_offset_pair(return_value,
                          String(last_match,
                                 ssubject.data() + offsets[0] - last_match,
                                 CopyString),
                          next_offset, NULL);
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
                              offsets[i<<1], NULL);
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
          if (re_bump == NULL) {
            int dummy;
            if ((re_bump = pcre_get_compiled_regex("/./us", &extra_bump,
                                                   &dummy)) == NULL) {
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

  start_offset = last_match - ssubject.data(); /* the offset might have been incremented, but without further successful matches */
  if (!no_empty || start_offset < ssubject.size()) {
    if (offset_capture) {
      /* Add the last (match, offset) pair to the return value */
      add_offset_pair(return_value,
                      ssubject.substr(start_offset),
                      start_offset, NULL);
    } else {
      /* Add the last piece to the return value */
      return_value.append
        (String(last_match, ssubject.data() + ssubject.size() - last_match,
                CopyString));
    }
  }

  /* Clean up */
  free(offsets);
  return return_value;
}

///////////////////////////////////////////////////////////////////////////////

String preg_quote(CStrRef str, CStrRef delimiter /* = null_string */) {
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
  return s_pcre_local->error_code;
}

///////////////////////////////////////////////////////////////////////////////
// regexec

static void php_reg_eprint(int err, regex_t *re) {
  char *buf = NULL, *message = NULL;
  size_t len;
  size_t buf_len;

#ifdef REG_ITOA
  /* get the length of the message */
  buf_len = regerror(REG_ITOA | err, re, NULL, 0);
  if (buf_len) {
    buf = (char *)malloc(buf_len);
    if (!buf) return; /* fail silently */
    /* finally, get the error message */
    regerror(REG_ITOA | err, re, buf, buf_len);
  }
#else
  buf_len = 0;
#endif
  len = regerror(err, re, NULL, 0);
  if (len) {
    message = (char *)malloc(buf_len + len + 2);
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
  if (buf) free(buf);
  if (message) free(message);
}

Variant php_split(CStrRef spliton, CStrRef str, int count, bool icase) {
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
