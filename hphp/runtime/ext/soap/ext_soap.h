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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/soap/soap.h"
namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct SoapServer : SystemLib::ClassLoader<"SoapServer"> {
  SoapServer();

  int                        m_type;
  soapFunctions              m_soap_functions;
  soapClass                  m_soap_class;
  Object                     m_soap_object;
  String                     m_actor;
  String                     m_uri;
  int                        m_version;
  sdl                       *m_sdl;
  xmlCharEncodingHandlerPtr  m_encoding;
  Array                      m_server_classmap;
  encodeMap                 *m_typemap;
  int                        m_features;
  Array                      m_soap_headers;
  int                        m_send_errors;
};

///////////////////////////////////////////////////////////////////////////////

struct SoapClient : SystemLib::ClassLoader<"SoapClient"> {
  SoapClient();

  int                         m_soap_version;
  sdl                        *m_sdl;
  xmlCharEncodingHandlerPtr   m_encoding;
  encodeMap                  *m_typemap;
  Array                       m_client_classmap;
  int                         m_features;
  String                      m_uri;
  String                      m_location;
  int                         m_style;
  int                         m_use;
  String                      m_login;
  String                      m_password;
  int                         m_authentication;
  bool                        m_digest;
  String                      m_proxy_host;
  int                         m_proxy_port;
  String                      m_proxy_login;
  String                      m_proxy_password;
  String                      m_proxy_ssl_cert_path;
  String                      m_proxy_ssl_key_path;
  String                      m_proxy_ssl_ca_bundle;
  int                         m_connection_timeout;
  int                         m_max_redirect;
  bool                        m_use11;
  String                      m_user_agent;
  int                         m_ssl_method = -1;
  int                         m_compression;
  Variant                     m_default_headers;
  Array                       m_cookies;
  bool                        m_exceptions;
  Variant                     m_soap_fault;
  bool                        m_trace;
  Variant                     m_last_request;
  Variant                     m_last_response;
  Variant                     m_last_request_headers;
  Variant                     m_last_response_headers;

  Array                       m_stream_context_options;
};

///////////////////////////////////////////////////////////////////////////////

extern const StaticString
  s_enc_type, s_enc_value, s_enc_stype,
  s_enc_ns, s_enc_name, s_enc_namens;

struct SoapVar : SystemLib::ClassLoader<"SoapVar"> {
  static int64_t getEncType(ObjectData* obj) {
    auto enc_type = obj->o_get(s_enc_type, false);
    if (!enc_type.isInitialized()) {
      raise_error("Encoding: SoapVar has no 'enc_type' property");
      not_reached();
      assertx(false);
    }
    return enc_type.toInt64();
  }

  static void setEncType(ObjectData* obj, int64_t t) {
    obj->setProp(nullctx, s_enc_type.get(), make_tv<KindOfInt64>(t));
  }

  static Variant getEncValue(ObjectData* obj) {
    return obj->o_get(s_enc_value, false);
  }

  static void setEncValue(ObjectData* obj, const Variant& val) {
    obj->setProp(nullctx, s_enc_value.get(), *val.asTypedValue());
  }

#define X(Name, str_name) \
  static void setEnc##Name(ObjectData* obj, const String& str) { \
    if (str.isNull()) { \
      obj->setProp(nullctx, s_enc_##str_name.get(), make_tv<KindOfNull>()); \
    } else { \
      obj->setProp(nullctx, s_enc_##str_name.get(), str.asTypedValue()); \
    } \
  } \
  static String getEnc##Name(ObjectData* obj) { \
    return getStrValue(obj, s_enc_##str_name); \
  }

  X(SType, stype)
  X(NS, ns)
  X(Name, name)
  X(NameNS, namens)

#undef X

 private:
  static String getStrValue(ObjectData* obj, const String& prop) {
    auto str = obj->o_get(prop, false);
    if (str.isString()) {
      return str.toString();
    } else {
      return String();
    }
  }
};

///////////////////////////////////////////////////////////////////////////////

struct SoapParam : SystemLib::ClassLoader<"SoapParam"> {
  String                      m_name;
  String                      m_data;
};

///////////////////////////////////////////////////////////////////////////////

struct SoapHeader : SystemLib::ClassLoader<"SoapHeader"> {
  String                      m_namespace;
  String                      m_name;
  Variant                     m_data;
  bool                        m_mustUnderstand;
  Variant                     m_actor;
};

///////////////////////////////////////////////////////////////////////////////
}
