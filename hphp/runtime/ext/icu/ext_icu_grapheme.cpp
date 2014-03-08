#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/ubrk.h>
#include <unicode/ucol.h>
#include <unicode/usearch.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////

#define CHECK_CONVERR(error) \
  if (U_FAILURE(error)) { \
    s_intl_error->setError(error, \
                           "Error converting input string to UTF-16"); \
    return false; \
  }

enum GraphemeExtractType {
  MIN = 0,
  COUNT = 0,
  MAXBYTES = 1,
  MAXCHARS = 2,
  MAX = 2,
};

inline bool outside_string(int64_t offset, int32_t max_len) {
  return (offset <= INT32_MIN) || \
         (offset >  INT32_MAX) || \
        ((offset < 0) ? (-offset > (long) max_len)
                      : (offset >= (long) max_len));
}

inline bool is_ascii(const String& str) {
  int len = str.size();
  const unsigned char *s = (const unsigned char*)str.c_str();
  while (len--) {
    if (s[len] & 0x80) return false;
  }
  return true;
}

inline UBreakIterator* get_break_iterator(const UChar* str, int32_t len) {
  UErrorCode error = U_ZERO_ERROR;
  UBreakIterator *bi = ubrk_open(UBRK_CHARACTER, nullptr, str, len, &error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Failed to instantiate break iterator");
    return nullptr;
  }
  return bi;
}

inline int64_t grapheme_count_graphemes(UBreakIterator *bi,
                                        const UChar *ustr,
                                        int32_t len) {
  UErrorCode error = U_ZERO_ERROR;
  ubrk_setText(bi, ustr, len, &error);
  int64_t ret = 0;
  while (ubrk_next(bi) != UBRK_DONE) ++ret;
  return ret;
}

inline int32_t grapheme_get_haystack_offset(UBreakIterator *bi,
                                            int32_t offset) {
  if (offset == 0) return 0;

  int32_t (*iter_op)(UBreakIterator* bi);
  if (offset > 0) {
    iter_op = ubrk_next;
  } else {
    ubrk_last(bi);
    iter_op = ubrk_previous;
    offset = -offset;
  }

  int32_t pos = 0;
  while ((pos != UBRK_DONE) && offset) {
    if ((pos = iter_op(bi)) != UBRK_DONE) {
      --offset;
    }
  }
  return offset ? -1 : pos;
}

static Variant grapheme_do_strpos(const String& haystack,
                                  const String& needle,
                                  int64_t &pos,
                                  int64_t offset,
                                  bool case_insensitive,
                                  bool reverse) {
  if (outside_string(offset, haystack.size())) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "grapheme_strpos: Offset not contained in string");
    return false;
  }
  if (needle.empty()) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "grapheme_strpos: Empty delimiter");
    return false;
  }

  // Fast-path, if the needles just not there,
  // or if it is and the haystack is ascii
  // (meaning the needle must have been as well)
  // Then we can skip the ICU overhead and return
  pos = reverse ?
    haystack.rfind(needle, offset, !case_insensitive) :
    haystack.find( needle, offset, !case_insensitive);
  if (pos < 0) return false;
  if (is_ascii(haystack)) return pos;

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString haystack16(u16(haystack, error));
  CHECK_CONVERR(error);

  error = U_ZERO_ERROR;
  icu::UnicodeString needle16(u16(needle, error));
  CHECK_CONVERR(error);

  auto bi = get_break_iterator(haystack16.getBuffer(), haystack16.length());
  if (!bi) return false;
  SCOPE_EXIT { ubrk_close(bi); };

  error = U_ZERO_ERROR;
  auto src = usearch_open(needle16.getBuffer(), needle16.length(),
                          haystack16.getBuffer(), haystack16.length(),
                          "", bi, &error);
  if (!src || U_FAILURE(error)) {
    s_intl_error->setError(error, "Error creating search object");
    return false;
  }
  SCOPE_EXIT { usearch_close(src); };

  if (case_insensitive) {
    auto coll = usearch_getCollator(src);
    error = U_ZERO_ERROR;
    ucol_setAttribute(coll, UCOL_STRENGTH, UCOL_SECONDARY, &error);
    if (U_FAILURE(error)) {
      s_intl_error->setError(error, "Error setting collation strength");
    }
    usearch_reset(src);
  }

  int32_t offset_pos = 0;
  if (offset != 0) {
    offset_pos = grapheme_get_haystack_offset(bi, offset);
    if (offset_pos < 0) {
      s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR, "Invalid search offset");
      return false;
    }
    error = U_ZERO_ERROR;
    usearch_setOffset(src, offset_pos, &error);
    if (U_FAILURE(error)) {
      s_intl_error->setError(error, "invalid search offset");
      return false;
    }
  }

  error = U_ZERO_ERROR;
  pos = reverse ? usearch_last(src, &error) : usearch_next(src, &error);
  if (pos < offset_pos) pos = USEARCH_DONE;
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Error looking up string");
    return false;
  }
  if (pos != USEARCH_DONE && ubrk_isBoundary(bi, pos)) {
        return grapheme_count_graphemes(bi, haystack16.getBuffer(), pos);
  }
  return false;
}

inline Variant grapheme_do_strpos(const String& haystack,
                                  const String& needle,
                                  int64_t offset,
                                  bool case_insensitive,
                                  bool reverse) {
  int64_t dummy;
  return grapheme_do_strpos(haystack, needle, dummy,
                            offset, case_insensitive, reverse);
}

static Variant grapheme_do_strstr(const String& haystack,
                                               const String& needle,
                                               bool before_needle = false,
                                               bool case_insensitive = false) {
  int64_t gpos;
  auto vpos = grapheme_do_strpos(haystack, needle, gpos, 0,
                                 case_insensitive, false);
  if (!vpos.isInteger()) {
    return vpos;
  }
  const char *s = haystack.c_str();
  int64_t ret = 0;
  U8_FWD_N(s, ret, haystack.size(), gpos);
  if (before_needle) {
    return haystack.substr(0, ret);
  } else {
    return haystack.substr(ret);
  }
}

/////////////////////////////////////////////////////////////////////////////

inline int32_t grapheme_extract_count_iter(UBreakIterator *bi,
                                           const char *p, int32_t len,
                                           int32_t size) {
  int32_t ret = 0, pos = 0;

  while (size--) {
    int32_t npos = ubrk_next(bi);
    if (npos == UBRK_DONE) break;
    pos = npos;
  }

  U8_FWD_N(p, ret, len, pos);
  return ret;
}

template<GraphemeExtractType T>
inline int32_t grapheme_extract_max_iter(UBreakIterator *bi,
                                         const char *p, int32_t len,
                                         int32_t size) {
  static_assert((T == GraphemeExtractType::MAXCHARS) ||
                (T == GraphemeExtractType::MAXBYTES),
                "grapheme_extract_max_iter only specializes MAXCHARS/BYTES");
  int32_t ret = 0, prev_ret = 0;
  int32_t pos = 0, prev_pos = 0;
  while (true) {
    if ((pos = ubrk_next(bi)) == UBRK_DONE) return ret;
    if ((T == GraphemeExtractType::MAXCHARS) && (pos > size)) return ret;
    prev_ret = ret;
    U8_FWD_N(p, ret, len, pos - prev_pos);
    if ((T == GraphemeExtractType::MAXBYTES) && (ret > size)) return prev_ret;
    if (UNLIKELY(prev_ret == ret)) return ret;
    prev_pos = pos;
  }
  not_reached();
}

static Variant HHVM_FUNCTION(grapheme_extract, const String& haystack,
                                               int64_t size,
                                               int64_t extract_type,
                                               int64_t start,
                                               VRefParam next) {
  next = start;
  if ((extract_type < GraphemeExtractType::MIN) ||
      (extract_type > GraphemeExtractType::MAX)) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "grapheme_extract: unknown extract type param");
    return false;
  }
  if ((start < 0) || (start > INT32_MAX) || (start >= haystack.size())) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "grapheme_extract: start not contained in string");
    return false;
  }
  if ((size < 0) || (size > INT32_MAX)) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "grapheme_extract: size is invalid");
    return false;
  }
  if (size == 0) {
    return empty_string;
  }

  const char *p = haystack.c_str() + start;
  int32_t len = haystack.size() - start;
  /* just in case pstr points in the middle of a character,
   * move forward to the start of the next char */
  if (!UTF8_IS_SINGLE(*p) && !U8_IS_LEAD(*p) ) {
    const char *e = p + len;

    while ( !UTF8_IS_SINGLE(*p) && !U8_IS_LEAD(*p) ) {
      if (++p >= e) {
        s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                               "grapheme_extract: invalid input string");
        return false;
      }
    }
    len = e - p;
  }

  /* if the string is all ASCII up to size+1 - or str_len whichever
   * is first - then we are done. (size + 1 because the size-th character
   *  might be the beginning of a grapheme cluster)
   */

  if (is_ascii(String(p, ((size + 1) < len) ? (size + 1) : len, CopyString))) {
    int32_t nsize = (size < len) ? size : len;
    next = start + nsize;
    return String(p, nsize, CopyString);
  }

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString chunk16(u16(p, len, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Error converting input string to UTF-16");
    return false;
  }
  auto bi = get_break_iterator(chunk16.getBuffer(), chunk16.length());
  SCOPE_EXIT { ubrk_close(bi); };
  int32_t pos = 0;
  switch (extract_type) {
    case GraphemeExtractType::COUNT:
      pos = grapheme_extract_count_iter(bi, p, len, size);
      break;
    case GraphemeExtractType::MAXBYTES:
      pos = grapheme_extract_max_iter<GraphemeExtractType::MAXBYTES>
                                     (bi, p, len, size);
      break;
    case GraphemeExtractType::MAXCHARS:
      pos = grapheme_extract_max_iter<GraphemeExtractType::MAXCHARS>
                                     (bi, p, len, size);
      break;
    default:
      not_reached();
  }

  next = start + pos;
  return String(p, pos, CopyString);
}

static Variant HHVM_FUNCTION(grapheme_stripos, const String& haystack,
                                               const String& needle,
                                               int64_t offset /*= 0 */) {
  return grapheme_do_strpos(haystack, needle, offset, true, false);
}


static Variant HHVM_FUNCTION(grapheme_stristr, const String& haystack,
                                               const String& needle,
                                               bool before_needle /*=false*/) {
  return grapheme_do_strstr(haystack, needle, before_needle, true);
}

static Variant HHVM_FUNCTION(grapheme_strlen, const String& str) {
  if (is_ascii(str)) {
    return str.size();
  }
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString str16(u16(str, error));
  CHECK_CONVERR(error);
  auto bi = get_break_iterator(str16.getBuffer(), str16.length());
  if (!bi) return false;
  SCOPE_EXIT { ubrk_close(bi); };
  int64_t retlen = 0;
  while (ubrk_next(bi) != UBRK_DONE) ++retlen;
  return retlen;
}

static Variant HHVM_FUNCTION(grapheme_strpos, const String& haystack,
                                              const String& needle,
                                              int64_t offset /* = 0 */) {
  return grapheme_do_strpos(haystack, needle, offset, false, false);
}

static Variant HHVM_FUNCTION(grapheme_strrpos, const String& haystack,
                                               const String& needle,
                                               int64_t offset /*= 0 */) {
  return grapheme_do_strpos(haystack, needle, offset, false, true);
}

static Variant HHVM_FUNCTION(grapheme_strripos, const String& haystack,
                                                const String& needle,
                                                int64_t offset /*= 0 */) {
  return grapheme_do_strpos(haystack, needle, offset, true, true);
}

static Variant HHVM_FUNCTION(grapheme_strstr, const String& haystack,
                                              const String& needle,
                                              bool before_needle /*=false */) {
  return grapheme_do_strstr(haystack, needle, before_needle, false);
}

static Variant HHVM_FUNCTION(grapheme_substr, const String& str,
                                              int64_t start,
                                              const Variant& len /*= NULL */) {
  if (outside_string(start, str.size())) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "grapheme_substr: start not contained in string");
    return false;
  }

  if (is_ascii(str)) {
    if (len.isNull()) {
      return str.substr(start);
    } else {
      return str.substr(start, len.toInt64());
    }
  }

  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString str16(u16(str, error));
  CHECK_CONVERR(error);
  auto bi = get_break_iterator(str16.getBuffer(), str16.length());
  if (!bi) return false;
  SCOPE_EXIT { ubrk_close(bi); };

  int32_t start_pos = 0;
  if (start) {
    int32_t (*iter_func)(UBreakIterator *);
    if (start > 0) {
      iter_func = ubrk_next;
    } else {
      iter_func = ubrk_previous;
      ubrk_last(bi);
      start = -start;
    }
    while (start && ((start_pos = iter_func(bi)) != UBRK_DONE)) --start;
  }

  if (start || (start_pos < 0) ||
      (start_pos >= str16.length()) ||
      (start_pos == UBRK_DONE)) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "grapheme_substr: start not contained in string");
    return false;
  }

  if (len.isNull()) {
    const char *s = str.c_str();
    int64_t ret = 0;
    U8_FWD_N(s, ret, str.size(), start_pos);
    return str.substr(ret);
  }

  int64_t length = len.toInt64();
  if (!length) {
    // we've validated start, we can return "" now
    return empty_string;
  }
  int32_t end_pos = start_pos;
  int32_t (*iter_func)(UBreakIterator *);
  if (length > 0) {
    iter_func = ubrk_next;
  } else {
    iter_func = ubrk_previous;
    ubrk_last(bi);
    length = -length;
  }
  while(length--) {
    end_pos = iter_func(bi);
    if (UBRK_DONE == end_pos) {
      break;
    }
  }
  if (end_pos == UBRK_DONE) {
    if (iter_func == ubrk_previous) {
      s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                             "grapheme_substr: length not contained in string");
      return false;
    }
    // Asked for more than whole string, treat like len.isNull case
    const char *s = str.c_str();
    int64_t ret = 0;
    U8_FWD_N(s, ret, str.size(), start_pos);
    return str.substr(ret);
  }

  if (end_pos < start_pos) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "grapheme_substr: length not contained in string");
    return false;
  } else {
    const char *s = str.c_str();
    int64_t ret_start = 0, ret_len = 0;
    U8_FWD_N(s, ret_start, str.size(), start_pos);
    U8_FWD_N(s, ret_len, str.size() - ret_start, end_pos - start_pos);
    return str.substr(ret_start, ret_len);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Extension

#define GRAPHEME_CONST(v) Native::registerConstant<KindOfInt64>( \
                            s_GRAPHEME_EXTR_##v.get(), \
                            GraphemeExtractType::v);

const StaticString
  s_GRAPHEME_EXTR_COUNT("GRAPHEME_EXTR_COUNT"),
  s_GRAPHEME_EXTR_MAXBYTES("GRAPHEME_EXTR_MAXBYTES"),
  s_GRAPHEME_EXTR_MAXCHARS("GRAPHEME_EXTR_MAXCHARS");

void IntlExtension::initGrapheme() {
  GRAPHEME_CONST(COUNT);
  GRAPHEME_CONST(MAXBYTES);
  GRAPHEME_CONST(MAXCHARS);

  HHVM_FE(grapheme_extract);
  HHVM_FE(grapheme_stripos);
  HHVM_FE(grapheme_stristr);
  HHVM_FE(grapheme_strlen);
  HHVM_FE(grapheme_strpos);
  HHVM_FE(grapheme_strripos);
  HHVM_FE(grapheme_strrpos);
  HHVM_FE(grapheme_strstr);
  HHVM_FE(grapheme_substr);

  loadSystemlib("icu_grapheme");
}

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
