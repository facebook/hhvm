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

#include "hphp/runtime/ext/url/ext_url.h"
#include <set>
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/ext/curl/ext_curl.h"
#include "hphp/runtime/ext/pcre/ext_pcre.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/std/ext_std_classobj.h"
#include "hphp/runtime/ext/std/ext_std_options.h"
#include "hphp/runtime/ext/string/ext_string.h"

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

String HHVM_FUNCTION(base64_encode, const String& data) {
  return StringUtil::Base64Encode(data);
}

Variant HHVM_FUNCTION(get_headers, const String& url, int64_t format /* = 0 */) {
  Variant c = HHVM_FN(curl_init)();
  HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_URL, url);
  HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_RETURNTRANSFER, true);
  HHVM_FN(curl_setopt)(c.toResource(), CURLOPT_HEADER, 1);
  Variant res = HHVM_FN(curl_exec)(c.toResource());
  if (same(res, false)) {
    return false;
  }

  String response = res.toString();
  int pos = response.find("\r\n\r\n");
  if (pos != String::npos) {
    response = response.substr(0, pos);
  }

  Array ret = HHVM_FN(explode)("\r\n", response).toArray();
  if (!format) {
    return ret;
  }

  Array assoc;
  for (ArrayIter iter(ret); iter; ++iter) {
    Array tokens =
      HHVM_FN(explode)(": ", iter.second().toString(), 2).toArray();
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
                    bool /*use_include_path*/ /* = false */) {
  auto const f = HHVM_FN(file_get_contents)(filename).toString();

  Variant matches;
  preg_match_all("/<meta\\s+name=\"(.*?)\"\\s+content=\"(.*?)\".*?>/s",
                 f, &matches, PREG_SET_ORDER);

  Array ret = Array::CreateDict();
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
  void *id = varr.isArray() ?
    (void*)varr.getArrayData() : (void*)varr.getObjectData();
  if (!seen_arrs.insert(id).second) {
    return; // recursive
  }

  // Allow multiple non-recursive references to the same array/object
  SCOPE_EXIT { seen_arrs.erase(id); };

  Array arr;
  if (varr.is(KindOfObject)) {
    Object o = varr.toObject();
    arr = o->isCollection()
      ? varr.toArray()
      : HHVM_FN(get_object_vars(o));
  } else {
    arr = varr.toArray();
  }

  for (ArrayIter iter(arr); iter; ++iter) {
    Variant data = iter.second();
    if (data.isNull() || data.isResource()) continue;

    auto const key = iter.first().toString();
    bool numeric = key.isNumeric();

    if (data.isArray() || data.is(KindOfObject)) {
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
                       arg_sep, encode_plus);
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

const StaticString s_arg_separator_output("&");

Variant HHVM_FUNCTION(http_build_query, const Variant& formdata,
                           const Variant& numeric_prefix /* = null_string */,
                           const String& arg_separator /* = null_string */,
                           int64_t enc_type /* = k_PHP_QUERY_RFC1738 */) {
  if (!formdata.isArray() && !formdata.is(KindOfObject)) {
    raise_invalid_argument_warning("formdata: (need Array or Object)");
    return false;
  }

  String arg_sep;
  if (arg_separator.empty()) {
    arg_sep = s_arg_separator_output;
  } else {
    arg_sep = arg_separator;
  }

  StringBuffer ret(1024);
  std::set<void*> seen_arrs;

  String num_prefix;
  if (!numeric_prefix.isNull()) {
    num_prefix = numeric_prefix.asCStrRef();
  }
  url_encode_array(ret, formdata, seen_arrs,
                   num_prefix, String(), String(), arg_sep,
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
  if (!resource.name.isNull()) {                        \
    return resource.name;                               \
  }                                                     \

#define SET_COMPONENT(name)                                             \
  if (!resource.name.isNull()) {                                        \
    ret.set(s_ ## name, resource.name);                                 \
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
    return init_null();
  }

  DictInit ret(resource.port ? 8 : 7);
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
  return ret.toVariant();
}

StringRet HHVM_FUNCTION(rawurldecode, const String& str) {
  return StringUtil::UrlDecode(str, false);
}

StringRet HHVM_FUNCTION(rawurlencode, const String& str) {
  return StringUtil::UrlEncode(str, false);
}

StringRet HHVM_FUNCTION(urldecode, const String& str) {
  return StringUtil::UrlDecode(str, true);
}

StringRet HHVM_FUNCTION(urlencode, const String& str) {
  return StringUtil::UrlEncode(str, true);
}

///////////////////////////////////////////////////////////////////////////////

struct StandardURLExtension final : Extension {
  StandardURLExtension() : Extension("url", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleInit() override {
    HHVM_RC_INT(PHP_URL_SCHEME, k_PHP_URL_SCHEME);
    HHVM_RC_INT(PHP_URL_HOST, k_PHP_URL_HOST);
    HHVM_RC_INT(PHP_URL_PORT, k_PHP_URL_PORT);
    HHVM_RC_INT(PHP_URL_USER, k_PHP_URL_USER);
    HHVM_RC_INT(PHP_URL_PASS, k_PHP_URL_PASS);
    HHVM_RC_INT(PHP_URL_PATH, k_PHP_URL_PATH);
    HHVM_RC_INT(PHP_URL_QUERY, k_PHP_URL_QUERY);
    HHVM_RC_INT(PHP_URL_FRAGMENT, k_PHP_URL_FRAGMENT);
    HHVM_RC_INT(PHP_QUERY_RFC1738, k_PHP_QUERY_RFC1738);
    HHVM_RC_INT(PHP_QUERY_RFC3986, k_PHP_QUERY_RFC3986);
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
  }
} s_standardurl_extension;

///////////////////////////////////////////////////////////////////////////////
}
