#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/runtime-error.h"

#include <fribidi/fribidi.h>

#define emalloc HPHP::req::malloc
#define efree HPHP::req::free

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
  logical_ustr = (FriBidiChar*) emalloc(sizeof(FriBidiChar) * logical_str_len);
  ustr_len = fribidi_charset_to_unicode(
    (FriBidiCharSet)charset,
    logical_str.c_str(),
    logical_str_len,
    logical_ustr
  );

  // Visualize the Unicode string
  base_direction = direction;
  visual_ustr = (FriBidiChar*) emalloc(sizeof(FriBidiChar) * ustr_len);
  status = fribidi_log2vis(
    logical_ustr, ustr_len, &base_direction,
    visual_ustr, nullptr, nullptr, nullptr);
  efree(logical_ustr);

  // Return false if FriBidi failed
  if (status == 0) {
    efree(visual_ustr);
    return false;
  }

  // Convert back from internal Unicode to original character set
  visual_str_len = 4 * ustr_len;
  visual_str = (char *) emalloc(sizeof(char) * visual_str_len);
  visual_str_len = fribidi_unicode_to_charset(
    (FriBidiCharSet)charset, visual_ustr, ustr_len, visual_str);
  efree(visual_ustr);

  String result(visual_str, visual_str_len, CopyString);
  efree(visual_str);

  return result;
}

static Array HHVM_FUNCTION(
  fribidi_charset_info,
  int64_t charset
) {
  Array result;

  result.add(
    String("name"),
    String((char *)fribidi_char_set_name((FriBidiCharSet)charset))
  );
  result.add(
    String("title"),
    String((char *)fribidi_char_set_title((FriBidiCharSet)charset))
  );

  if (fribidi_char_set_desc((FriBidiCharSet)charset)) {
    result.add(
      String("desc"),
      String((char *)fribidi_char_set_desc((FriBidiCharSet)charset))
    );
  }

  return result;
}

static Array HHVM_FUNCTION(
  fribidi_get_charsets
) {
  Array result;

  result.add(FRIBIDI_CHAR_SET_UTF8, s_FRIBIDI_CHARSET_UTF8.get());
  result.add(FRIBIDI_CHAR_SET_ISO8859_6, s_FRIBIDI_CHARSET_8859_6.get());
  result.add(FRIBIDI_CHAR_SET_ISO8859_8, s_FRIBIDI_CHARSET_8859_8.get());
  result.add(FRIBIDI_CHAR_SET_CP1255, s_FRIBIDI_CHARSET_CP1255.get());
  result.add(FRIBIDI_CHAR_SET_CP1256, s_FRIBIDI_CHARSET_CP1256.get());
  result.add(FRIBIDI_CHAR_SET_CAP_RTL, s_FRIBIDI_CHARSET_CAP_RTL.get());

  return result;
}

class FribidiExtension final : public Extension {
public:
  FribidiExtension() : Extension("fribidi") {}
  void moduleInit() override {
    // Charsets
    Native::registerConstant<KindOfInt64>(
      s_FRIBIDI_CHARSET_UTF8.get(),
      FRIBIDI_CHAR_SET_UTF8
    );
    Native::registerConstant<KindOfInt64>(
      s_FRIBIDI_CHARSET_8859_6.get(),
      FRIBIDI_CHAR_SET_ISO8859_6
    );
    Native::registerConstant<KindOfInt64>(
      s_FRIBIDI_CHARSET_8859_8.get(),
      FRIBIDI_CHAR_SET_ISO8859_8
    );
    Native::registerConstant<KindOfInt64>(
      s_FRIBIDI_CHARSET_CP1255.get(),
      FRIBIDI_CHAR_SET_CP1255
    );
    Native::registerConstant<KindOfInt64>(
      s_FRIBIDI_CHARSET_CP1256.get(),
      FRIBIDI_CHAR_SET_CP1256
    );
    Native::registerConstant<KindOfInt64>(
      s_FRIBIDI_CHARSET_CAP_RTL.get(),
      FRIBIDI_CHAR_SET_CAP_RTL
    );

    // Directions
    Native::registerConstant<KindOfInt64>(
      s_FRIBIDI_AUTO.get(),
      FRIBIDI_PAR_ON
    );
    Native::registerConstant<KindOfInt64>(
      s_FRIBIDI_LTR.get(),
      FRIBIDI_PAR_LTR
    );
    Native::registerConstant<KindOfInt64>(
      s_FRIBIDI_RTL.get(),
      FRIBIDI_PAR_RTL
    );
    Native::registerConstant<KindOfInt64>(
      s_FRIBIDI_WLTR.get(),
      FRIBIDI_PAR_WLTR
    );
    Native::registerConstant<KindOfInt64>(
      s_FRIBIDI_WRTL.get(),
      FRIBIDI_PAR_WRTL
    );

    HHVM_FE(fribidi_log2vis);
    HHVM_FE(fribidi_charset_info);
    HHVM_FE(fribidi_get_charsets);
    loadSystemlib();
  }
} s_fribidi_extension;

}
