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

#include <runtime/ext/ext_url.h>
#include <runtime/base/string_util.h>
#include <runtime/base/zend/zend_url.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/ext/ext_curl.h>
#include <runtime/ext/ext_string.h>
#include <runtime/ext/ext_file.h>
#include <runtime/ext/ext_preg.h>
#include <runtime/ext/ext_options.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_base64_decode(CStrRef data, bool strict /* = false */) {
  String decoded = StringUtil::Base64Decode(data, strict);
  if (decoded.isNull()) {
    return false;
  }
  return decoded;
}

String f_base64_encode(CStrRef data) {
  String encoded = StringUtil::Base64Encode(data);
  if (encoded.isNull()) {
    return false;
  }
  return encoded;
}

Variant f_get_headers(CStrRef url, int format /* = 0 */) {
  Variant c = f_curl_init();
  f_curl_setopt(c, k_CURLOPT_URL, url);
  f_curl_setopt(c, k_CURLOPT_RETURNTRANSFER, true);
  f_curl_setopt(c, k_CURLOPT_HEADER, 1);
  Variant res = f_curl_exec(c);
  if (same(res, false)) {
    return false;
  }

  String response = res.toString();
  int pos = response.find("\r\n\r\n");
  if (pos != String::npos) {
    response = response.substr(0, pos);
  }

  Array ret = f_explode("\r\n", response);
  if (!format) {
    return ret;
  }

  Array assoc;
  for (ArrayIter iter(ret); iter; ++iter) {
    Array tokens = f_explode(": ", iter.second(), 2);
    if (tokens.size() == 2) {
      assoc.set(tokens[0], tokens[1]);
    } else {
      assoc.append(iter.second());
    }
  }
  return assoc;
}

static String normalize_variable_name(CStrRef name) {
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

Array f_get_meta_tags(CStrRef filename, bool use_include_path /* = false */) {
  String f = f_file_get_contents(filename);

  Variant matches;
  f_preg_match_all("/<meta\\s+name=\"(.*?)\"\\s+content=\"(.*?)\".*?>/s",
                   f, ref(matches), k_PREG_SET_ORDER);

  Array ret = Array::Create();
  for (ArrayIter iter(matches); iter; ++iter) {
    Array pair = iter.second();
    ret.set(normalize_variable_name(pair[1].toString()), pair[2]);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

static void url_encode_array(StringBuffer &ret, CArrRef arr,
                             std::set<void*> &seen_arrs,
                             CStrRef num_prefix, CStrRef key_prefix,
                             CStrRef key_suffix, CStrRef arg_sep) {
  if (seen_arrs.find((void*)arr.get()) != seen_arrs.end()) {
    return; // recursive
  }
  seen_arrs.insert((void*)arr.get());

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
      new_prefix += key_prefix;
      if (numeric) new_prefix += num_prefix;
      new_prefix += encoded;
      new_prefix += key_suffix;
      new_prefix += "%5B";
      url_encode_array(ret, data.toArray(), seen_arrs, String(),
                       new_prefix.detach(), String("%5D", AttachLiteral),
                       arg_sep);
    } else {
      if (!ret.empty()) {
        ret += arg_sep;
      }
      ret += key_prefix;
      if (numeric) {
        ret += num_prefix;
        ret += key;
      } else {
        ret += StringUtil::UrlEncode(key);
      }
      ret += key_suffix;
      ret += "=";
      if (data.isInteger() || data.is(KindOfBoolean)) {
        ret += String(data.toInt64());
      } else if (data.is(KindOfDouble)) {
        ret += String(data.toDouble());
      } else {
        ret += StringUtil::UrlEncode(data.toString());
      }
    }
  }
}

Variant f_http_build_query(CVarRef formdata,
                           CStrRef numeric_prefix /* = null_string */,
                           CStrRef arg_separator /* = null_string */) {
  if (!formdata.is(KindOfArray) && !formdata.is(KindOfObject)) {
    throw_invalid_argument("formdata: (need Array or Object)");
    return false;
  }

  String arg_sep;
  if (arg_separator.isNull()) {
    arg_sep = f_ini_get("arg_separator.output");
  } else {
    arg_sep = arg_separator;
  }

  StringBuffer ret(1024);
  std::set<void*> seen_arrs;
  url_encode_array(ret, formdata.toArray(), seen_arrs,
                   numeric_prefix, String(), String(), arg_sep);
  return ret.detach();
}

///////////////////////////////////////////////////////////////////////////////

#define RETURN_COMPONENT(name)                          \
  if (resource.name != NULL) {                          \
    String ret(resource.name, AttachString);            \
    resource.name = NULL;                               \
    return ret;                                         \
  }                                                     \

#define SET_COMPONENT(name)                                             \
  if (resource.name != NULL) {                                          \
    ret.set(#name, String(resource.name, AttachString));                \
    resource.name = NULL;                                               \
  }                                                                     \

#define PHP_URL_SCHEME 0
#define PHP_URL_HOST 1
#define PHP_URL_PORT 2
#define PHP_URL_USER 3
#define PHP_URL_PASS 4
#define PHP_URL_PATH 5
#define PHP_URL_QUERY 6
#define PHP_URL_FRAGMENT 7

Variant f_parse_url(CStrRef url, int component /* = -1 */) {
  Url resource;
  if (!url_parse(resource, url.data(), url.size())) {
    raise_notice("invalid url: %s", url.data());
    return false;
  }

  if (component > -1) {
    switch (component) {
    case PHP_URL_SCHEME:   RETURN_COMPONENT(scheme);   break;
    case PHP_URL_HOST:     RETURN_COMPONENT(host);     break;
    case PHP_URL_USER:     RETURN_COMPONENT(user);     break;
    case PHP_URL_PASS:     RETURN_COMPONENT(pass);     break;
    case PHP_URL_PATH:     RETURN_COMPONENT(path);     break;
    case PHP_URL_QUERY:    RETURN_COMPONENT(query);    break;
    case PHP_URL_FRAGMENT: RETURN_COMPONENT(fragment); break;
    case PHP_URL_PORT:
      if (resource.port) {
        return resource.port;
      }
      break;
    default:
      throw_invalid_argument("component: %d", component);
      return false;
    }
    return null;
  }

  Array ret;
  SET_COMPONENT(scheme);
  SET_COMPONENT(host);
  SET_COMPONENT(user);
  SET_COMPONENT(pass);
  SET_COMPONENT(path);
  SET_COMPONENT(query);
  SET_COMPONENT(fragment);
  if (resource.port) {
    ret.set("port", (int64)resource.port);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
