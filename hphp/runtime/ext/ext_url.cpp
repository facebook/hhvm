/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_url.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/ext/ext_curl.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/ext/ext_preg.h"
#include "hphp/runtime/ext/ext_options.h"

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
///////////////////////////////////////////////////////////////////////////////

Variant f_base64_decode(const String& data, bool strict /* = false */) {
  String decoded = StringUtil::Base64Decode(data, strict);
  if (decoded.isNull()) {
    return false;
  }
  return decoded;
}

String f_base64_encode(const String& data) {
  String encoded = StringUtil::Base64Encode(data);
  if (encoded.isNull()) {
    return false;
  }
  return encoded;
}

Variant f_get_headers(const String& url, int format /* = 0 */) {
  Variant c = f_curl_init();
  f_curl_setopt(c.toResource(), k_CURLOPT_URL, url);
  f_curl_setopt(c.toResource(), k_CURLOPT_RETURNTRANSFER, true);
  f_curl_setopt(c.toResource(), k_CURLOPT_HEADER, 1);
  Variant res = f_curl_exec(c.toResource());
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

Array f_get_meta_tags(const String& filename, bool use_include_path /* = false */) {
  String f = f_file_get_contents(filename);

  Variant matches;
  f_preg_match_all("/<meta\\s+name=\"(.*?)\"\\s+content=\"(.*?)\".*?>/s",
                   f, ref(matches), k_PREG_SET_ORDER);

  Array ret = Array::Create();
  for (ArrayIter iter(matches.toArray()); iter; ++iter) {
    Array pair = iter.second().toArray();
    ret.set(normalize_variable_name(pair[1].toString()), pair[2]);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

static void url_encode_array(StringBuffer &ret, CVarRef varr,
                             std::set<void*> &seen_arrs,
                             const String& num_prefix, const String& key_prefix,
                             const String& key_suffix, const String& arg_sep) {
  void *id = varr.is(KindOfArray) ?
    (void*)varr.getArrayData() : (void*)varr.getObjectData();
  if (!seen_arrs.insert(id).second) {
    return; // recursive
  }

  Array arr;
  if (varr.is(KindOfObject)) {
    Object o = varr.toObject();
    arr = (o.objectForCall()->isCollection()) ?
      varr.toArray() :
      f_get_object_vars(o).toArray();
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
        encoded = StringUtil::UrlEncode(key);
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
        ret.append(StringUtil::UrlEncode(key));
      }
      ret.append(key_suffix);
      ret.append("=");
      if (data.isInteger() || data.is(KindOfBoolean)) {
        ret.append(String(data.toInt64()));
      } else if (data.is(KindOfDouble)) {
        ret.append(String(data.toDouble()));
      } else {
        ret.append(StringUtil::UrlEncode(data.toString()));
      }
    }
  }
}

const StaticString s_arg_separator_output("arg_separator.output");

Variant f_http_build_query(CVarRef formdata,
                           const String& numeric_prefix /* = null_string */,
                           const String& arg_separator /* = null_string */) {
  if (!formdata.is(KindOfArray) && !formdata.is(KindOfObject)) {
    throw_invalid_argument("formdata: (need Array or Object)");
    return false;
  }

  String arg_sep;
  if (arg_separator.isNull()) {
    arg_sep = f_ini_get(s_arg_separator_output);
  } else {
    arg_sep = arg_separator;
  }

  StringBuffer ret(1024);
  std::set<void*> seen_arrs;
  url_encode_array(ret, formdata, seen_arrs,
                   numeric_prefix, String(), String(), arg_sep);
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

Variant f_parse_url(const String& url, int component /* = -1 */) {
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
      throw_invalid_argument("component: %d", component);
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

String f_rawurldecode(const String& str) {
  return StringUtil::UrlDecode(str, false);
}

String f_rawurlencode(const String& str) {
  return StringUtil::UrlEncode(str, false);
}

String f_urldecode(const String& str) {
  return StringUtil::UrlDecode(str, true);
}

String f_urlencode(const String& str) {
  return StringUtil::UrlEncode(str, true);
}

///////////////////////////////////////////////////////////////////////////////
}
