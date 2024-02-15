#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/array-init.h"

#include <fribidi/fribidi.h>

namespace HPHP {

static inline bool
_direction_is_valid(int64_t direction) {
  switch (direction) {
    case FRIBIDI_PAR_ON:
    case FRIBIDI_PAR_LTR:
    case FRIBIDI_PAR_RTL:
    case FRIBIDI_PAR_WLTR:
    case FRIBIDI_PAR_WRTL:
      return true;
  }

  return false;
}

static inline bool
_charset_is_valid(int64_t charset) {
  switch (charset) {
    case FRIBIDI_CHAR_SET_UTF8:
    case FRIBIDI_CHAR_SET_ISO8859_6:
    case FRIBIDI_CHAR_SET_ISO8859_8:
    case FRIBIDI_CHAR_SET_CP1255:
    case FRIBIDI_CHAR_SET_CP1256:
    case FRIBIDI_CHAR_SET_CAP_RTL:
      return true;
  }

  return false;
}

/* Charsets */
const StaticString s_FRIBIDI_CHARSET_UTF8("FRIBIDI_CHARSET_UTF8");
const StaticString s_FRIBIDI_CHARSET_8859_6("FRIBIDI_CHARSET_8859_6");
const StaticString s_FRIBIDI_CHARSET_8859_8("FRIBIDI_CHARSET_8859_8");
const StaticString s_FRIBIDI_CHARSET_CP1255("FRIBIDI_CHARSET_CP1255");
const StaticString s_FRIBIDI_CHARSET_CP1256("FRIBIDI_CHARSET_CP1256");
const StaticString s_FRIBIDI_CHARSET_CAP_RTL("FRIBIDI_CHARSET_CAP_RTL");

/* Directions */
const StaticString s_FRIBIDI_AUTO("FRIBIDI_AUTO");
const StaticString s_FRIBIDI_LTR("FRIBIDI_LTR");
const StaticString s_FRIBIDI_RTL("FRIBIDI_RTL");
const StaticString s_FRIBIDI_WLTR("FRIBIDI_WLTR");
const StaticString s_FRIBIDI_WRTL("FRIBIDI_WRTL");

static Variant HHVM_FUNCTION(
    fribidi_log2vis,
    const String& logical_str,
    int64_t direction,
    int64_t charset
  ) {

  char * visual_str;
  int logical_str_len, visual_str_len, ustr_len;
  FriBidiParType base_direction;
  FriBidiLevel status;
  FriBidiChar *logical_ustr, *visual_ustr;

  logical_str_len = logical_str.length();

  if (!_direction_is_valid(direction)) {
    raise_warning("Uknown direction.");
    return false;
  }

  if (!_charset_is_valid(charset)) {
    raise_warning("Uknown charset.");
    return false;
  }

  // Convert input string to internal Unicode
  logical_ustr =
    (FriBidiChar*) req::malloc_noptrs(sizeof(FriBidiChar) * logical_str_len);
  SCOPE_EXIT { req::free(logical_ustr); };

  ustr_len = fribidi_charset_to_unicode(
    (FriBidiCharSet)charset,
    logical_str.c_str(),
    logical_str_len,
    logical_ustr
  );

  // Visualize the Unicode string
  base_direction = direction;
  visual_ustr =
    (FriBidiChar*) req::malloc_noptrs(sizeof(FriBidiChar) * ustr_len);
  SCOPE_EXIT { req::free(visual_ustr); };
  status = fribidi_log2vis(
    logical_ustr, ustr_len, &base_direction,
    visual_ustr, nullptr, nullptr, nullptr);

  // Return false if FriBidi failed
  if (status == 0) {
    return false;
  }

  // Convert back from internal Unicode to original character set
  visual_str_len = 4 * ustr_len;
  visual_str = (char *) req::malloc_noptrs(sizeof(char) * visual_str_len);
  SCOPE_EXIT { req::free(visual_str); };

  visual_str_len = fribidi_unicode_to_charset(
    (FriBidiCharSet)charset, visual_ustr, ustr_len, visual_str);

  return String(visual_str, visual_str_len, CopyString);
}

static Array HHVM_FUNCTION(
  fribidi_charset_info,
  int64_t charset
) {
  Array result = Array::CreateDict();

  result.set(
    String("name"),
    String((char *)fribidi_char_set_name((FriBidiCharSet)charset))
  );
  result.set(
    String("title"),
    String((char *)fribidi_char_set_title((FriBidiCharSet)charset))
  );

  if (fribidi_char_set_desc((FriBidiCharSet)charset)) {
    result.set(
      String("desc"),
      String((char *)fribidi_char_set_desc((FriBidiCharSet)charset))
    );
  }

  return result;
}

static Array HHVM_FUNCTION(
  fribidi_get_charsets
) {
  return make_dict_array(
    FRIBIDI_CHAR_SET_UTF8,      Variant{s_FRIBIDI_CHARSET_UTF8.get()},
    FRIBIDI_CHAR_SET_ISO8859_6, Variant{s_FRIBIDI_CHARSET_8859_6.get()},
    FRIBIDI_CHAR_SET_ISO8859_8, Variant{s_FRIBIDI_CHARSET_8859_8.get()},
    FRIBIDI_CHAR_SET_CP1255,    Variant{s_FRIBIDI_CHARSET_CP1255.get()},
    FRIBIDI_CHAR_SET_CP1256,    Variant{s_FRIBIDI_CHARSET_CP1256.get()},
    FRIBIDI_CHAR_SET_CAP_RTL,   Variant{s_FRIBIDI_CHARSET_CAP_RTL.get()}
  );
}

struct FribidiExtension final : Extension {
  FribidiExtension() : Extension("fribidi", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleRegisterNative() override {
    // Charsets
    HHVM_RC_INT(FRIBIDI_CHARSET_UTF8, FRIBIDI_CHAR_SET_UTF8);
    HHVM_RC_INT(FRIBIDI_CHARSET_8859_6, FRIBIDI_CHAR_SET_ISO8859_6);
    HHVM_RC_INT(FRIBIDI_CHARSET_8859_8, FRIBIDI_CHAR_SET_ISO8859_8);
    HHVM_RC_INT(FRIBIDI_CHARSET_CP1255, FRIBIDI_CHAR_SET_CP1255);
    HHVM_RC_INT(FRIBIDI_CHARSET_CP1256, FRIBIDI_CHAR_SET_CP1256);
    HHVM_RC_INT(FRIBIDI_CHARSET_CAP_RTL, FRIBIDI_CHAR_SET_CAP_RTL);

    // Directions
    HHVM_RC_INT(FRIBIDI_AUTO, FRIBIDI_PAR_ON);
    HHVM_RC_INT(FRIBIDI_LTR, FRIBIDI_PAR_LTR);
    HHVM_RC_INT(FRIBIDI_RTL, FRIBIDI_PAR_RTL);
    HHVM_RC_INT(FRIBIDI_WLTR, FRIBIDI_PAR_WLTR);
    HHVM_RC_INT(FRIBIDI_WRTL, FRIBIDI_PAR_WRTL);

    HHVM_FE(fribidi_log2vis);
    HHVM_FE(fribidi_charset_info);
    HHVM_FE(fribidi_get_charsets);
  }
} s_fribidi_extension;

}
