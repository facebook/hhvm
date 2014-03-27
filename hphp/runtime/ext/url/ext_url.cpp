/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/url/ext_url.h"
#include <set>
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/ext/curl/ext_curl.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/ext/pcre/ext_pcre.h"
#include "hphp/runtime/ext/std/ext_std_options.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
const int64_t k_PHP_URL_SCHEME = 0;
const int64_t k_PHP_URL_HOST = 1;
const int64_t k_PHP_URL_PORT = 2;
const int64_t k_PHP_URL_USER = 3;
const int64_t k_PHP_URL_PASS = 4;
const int64_t k_PHP_URL_PATH = 5;
const int64_t k_PHP_URL_QUERY = 6;
const int64_t k_PHP_URL_FRAGMENT = 7;
const int64_t k_PHP_QUERY_RFC1738 = 1;
const int64_t k_PHP_QUERY_RFC3986 = 2;
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(base64_decode, const String& data,
                                     bool strict /* = false */) {
  String decoded = StringUtil::Base64Decode(data, strict);
  if (decoded.isNull()) {
    return false;
  }
  return decoded;
}

Variant HHVM_FUNCTION(base64_encode, const String& data) {
  String encoded = StringUtil::Base64Encode(data);
  if (encoded.isNull()) {
    return false;
  }
  return encoded;
}

Variant HHVM_FUNCTION(get_headers, const String& url, int format /* = 0 */) {
  Variant c = HHVM_FN(curl_init)();
  HHVM_FN(curl_setopt)(c.toResource(), k_CURLOPT_URL, url);
  HHVM_FN(curl_setopt)(c.toResource(), k_CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_setopt)(c.toResource(), k_CURLOPT_HEADER, 1);
  Variant res = HHVM_FN(curl_exec)(c.toResource());
  if (same(res, false)) {
    return false;
  }

  String response = res.toString();
  int pos = response.find("\r\n\r\n");
  if (pos != String::npos) {
    response = response.substr(0, pos);
  }

  Array ret = f_explode("\r\n", response).toArray();
  if (!format) {
    return ret;
  }

  Array assoc;
  for (ArrayIter iter(ret); iter; ++iter) {
    Array tokens = f_explode(": ", iter.second(), 2).toArray();
    if (tokens.size() == 2) {
      assoc.set(tokens[0], tokens[1]);
    } else {
      assoc.append(iter.second());
    }
  }
  return assoc;
 }

static String normalize_variable_name(const String& name) {
  StringBuffer sb;
  for (int i = 0; i < name.size(); i++) {
    char ch = name.charAt(i);
    if ((i > 0 && ch >= '0' && ch <= '9') ||
        (ch >= 'a' && ch <= 'z') ||
        (ch == '_')) {
      sb.append(ch);
    } else if (ch >= 'A' && ch <= 'Z') {
      sb.append((char)(ch + 'a' - 'A'));
    } else {
      sb.append('_');
    }
  }
  return sb.detach();
}

Array HHVM_FUNCTION(get_meta_tags, const String& filename,
                                   bool use_include_path /* = false */) {
  String f = f_file_get_contents(filename);

  Variant matches;
  HHVM_FN(preg_match_all)("/<meta\\s+name=\"(.*?)\"\\s+content=\"(.*?)\".*?>/s",
                          f, ref(matches), k_PREG_SET_ORDER);

  Array ret = Array::Create();
  for (ArrayIter iter(matches.toArray()); iter; ++iter) {
    Array pair = iter.second().toArray();
    ret.set(normalize_variable_name(pair[1].toString()), pair[2]);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

static void url_encode_array(StringBuffer &ret, const Variant& varr,
                             std::set<void*> &seen_arrs,
                             const String& num_prefix, const String& key_prefix,
                             const String& key_suffix, const String& arg_sep,
                             bool encode_plus = true) {
  void *id = varr.is(KindOfArray) ?
    (void*)varr.getArrayData() : (void*)varr.getObjectData();
  if (!seen_arrs.insert(id).second) {
    return; // recursive
  }

  Array arr;
  if (varr.is(KindOfObject)) {
    Object o = varr.toObject();
    arr = o->isCollection()
      ? varr.toArray()
      : f_get_object_vars(o).toArray();
  } else {
    arr = varr.toArray();
  }

  for (ArrayIter iter(arr); iter; ++iter) {
    Variant data = iter.second();
    if (data.isNull() || data.isResource()) continue;

    String key = iter.first();
    bool numeric = key.isNumeric();

    if (data.is(KindOfArray) || data.is(KindOfObject)) {
      String encoded;
      if (numeric) {
        encoded = key;
      } else {
        encoded = StringUtil::UrlEncode(key, encode_plus);
      }
      StringBuffer new_prefix(key_prefix.size() + num_prefix.size() +
                              encoded.size() + key_suffix.size() + 4);
      new_prefix.append(key_prefix);
      if (numeric) new_prefix.append(num_prefix);
      new_prefix.append(encoded);
      new_prefix.append(key_suffix);
      new_prefix.append("%5B");
      url_encode_array(ret, data, seen_arrs, String(),
                       new_prefix.detach(), String("%5D", CopyString),
                       arg_sep);
    } else {
      if (!ret.empty()) {
        ret.append(arg_sep);
      }
      ret.append(key_prefix);
      if (numeric) {
        ret.append(num_prefix);
        ret.append(key);
      } else {
        ret.append(StringUtil::UrlEncode(key, encode_plus));
      }
      ret.append(key_suffix);
      ret.append("=");
      if (data.isInteger() || data.is(KindOfBoolean)) {
        ret.append(String(data.toInt64()));
      } else if (data.is(KindOfDouble)) {
        ret.append(String(data.toDouble()));
      } else {
        ret.append(StringUtil::UrlEncode(data.toString(), encode_plus));
      }
    }
  }
}

const StaticString s_arg_separator_output("arg_separator.output");

Variant HHVM_FUNCTION(http_build_query, const Variant& formdata,
                           const String& numeric_prefix /* = null_string */,
                           const String& arg_separator /* = null_string */,
                           int enc_type /* = k_PHP_QUERY_RFC1738 */) {
  if (!formdata.is(KindOfArray) && !formdata.is(KindOfObject)) {
    throw_invalid_argument("formdata: (need Array or Object)");
    return false;
  }

  String arg_sep;
  if (arg_separator.empty()) {
    arg_sep = HHVM_FN(ini_get)(s_arg_separator_output);
  } else {
    arg_sep = arg_separator;
  }

  StringBuffer ret(1024);
  std::set<void*> seen_arrs;
  url_encode_array(ret, formdata, seen_arrs,
                   numeric_prefix, String(), String(), arg_sep,
                   enc_type != k_PHP_QUERY_RFC3986);
  return ret.detach();
}

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_scheme("scheme"),
  s_host("host"),
  s_user("user"),
  s_pass("pass"),
  s_path("path"),
  s_query("query"),
  s_fragment("fragment"),
  s_port("port");

#define RETURN_COMPONENT(name)                          \
  if (resource.name != NULL) {                          \
    String ret(resource.name, AttachString);            \
    resource.name = NULL;                               \
    return ret;                                         \
  }                                                     \

#define SET_COMPONENT(name)                                             \
  if (resource.name != NULL) {                                          \
    ret.set(s_ ## name, String(resource.name, AttachString));           \
    resource.name = NULL;                                               \
  }                                                                     \

Variant HHVM_FUNCTION(parse_url, const String& url,
                                 int64_t component /* = -1 */) {
  Url resource;
  if (!url_parse(resource, url.data(), url.size())) {
    return false;
  }

  if (component > -1) {
    switch (component) {
    case k_PHP_URL_SCHEME:   RETURN_COMPONENT(scheme);   break;
    case k_PHP_URL_HOST:     RETURN_COMPONENT(host);     break;
    case k_PHP_URL_USER:     RETURN_COMPONENT(user);     break;
    case k_PHP_URL_PASS:     RETURN_COMPONENT(pass);     break;
    case k_PHP_URL_PATH:     RETURN_COMPONENT(path);     break;
    case k_PHP_URL_QUERY:    RETURN_COMPONENT(query);    break;
    case k_PHP_URL_FRAGMENT: RETURN_COMPONENT(fragment); break;
    case k_PHP_URL_PORT:
      if (resource.port) {
        return resource.port;
      }
      break;
    default:
      raise_warning(
        "parse_url(): Invalid URL component identifier %" PRId64, component);
      return false;
    }
    return uninit_null();
  }

  ArrayInit ret(8);
  SET_COMPONENT(scheme);
  SET_COMPONENT(host);
  if (resource.port) {
    ret.set(s_port, (int64_t)resource.port);
  }
  SET_COMPONENT(user);
  SET_COMPONENT(pass);
  SET_COMPONENT(path);
  SET_COMPONENT(query);
  SET_COMPONENT(fragment);
  return ret.create();
}

String HHVM_FUNCTION(rawurldecode, const String& str) {
  return StringUtil::UrlDecode(str, false);
}

String HHVM_FUNCTION(rawurlencode, const String& str) {
  return StringUtil::UrlEncode(str, false);
}

String HHVM_FUNCTION(urldecode, const String& str) {
  return StringUtil::UrlDecode(str, true);
}

String HHVM_FUNCTION(urlencode, const String& str) {
  return StringUtil::UrlEncode(str, true);
}

///////////////////////////////////////////////////////////////////////////////

const StaticString s_PHP_URL_SCHEME("PHP_URL_SCHEME");
const StaticString s_PHP_URL_HOST("PHP_URL_HOST");
const StaticString s_PHP_URL_PORT("PHP_URL_PORT");
const StaticString s_PHP_URL_USER("PHP_URL_USER");
const StaticString s_PHP_URL_PASS("PHP_URL_PASS");
const StaticString s_PHP_URL_PATH("PHP_URL_PATH");
const StaticString s_PHP_URL_QUERY("PHP_URL_QUERY");
const StaticString s_PHP_URL_FRAGMENT("PHP_URL_FRAGMENT");
const StaticString s_PHP_QUERY_RFC1738("PHP_QUERY_RFC1738");
const StaticString s_PHP_QUERY_RFC3986("PHP_QUERY_RFC3986");

class StandardURLExtension : public Extension {
 public:
  StandardURLExtension() : Extension("url") {}
  virtual void moduleInit() {
    Native::registerConstant<KindOfInt64>(
      s_PHP_URL_SCHEME.get(), k_PHP_URL_SCHEME
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_URL_HOST.get(), k_PHP_URL_HOST
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_URL_PORT.get(), k_PHP_URL_PORT
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_URL_USER.get(), k_PHP_URL_USER
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_URL_PASS.get(), k_PHP_URL_PASS
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_URL_PATH.get(), k_PHP_URL_PATH
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_URL_QUERY.get(), k_PHP_URL_QUERY
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_URL_FRAGMENT.get(), k_PHP_URL_FRAGMENT
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_QUERY_RFC1738.get(), k_PHP_QUERY_RFC1738
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_QUERY_RFC3986.get(), k_PHP_QUERY_RFC3986
    );
    HHVM_FE(base64_decode);
    HHVM_FE(base64_encode);
    HHVM_FE(get_headers);
    HHVM_FE(get_meta_tags);
    HHVM_FE(http_build_query);
    HHVM_FE(parse_url);
    HHVM_FE(rawurldecode);
    HHVM_FE(rawurlencode);
    HHVM_FE(urldecode);
    HHVM_FE(urlencode);

    loadSystemlib();
  }
} s_standardurl_extension;

///////////////////////////////////////////////////////////////////////////////
}
