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

#ifndef incl_HPHP_EXT_SOAP_H_
#define incl_HPHP_EXT_SOAP_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/soap/soap.h"
namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(use_soap_error_handler,
                   bool handler = true);
bool HHVM_FUNCTION(is_soap_fault,
                   const Variant& fault);
int64_t HHVM_FUNCTION(_soap_active_version);

///////////////////////////////////////////////////////////////////////////////

class SoapServer {
public:
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
  Array                      m_classmap;
  encodeMap                 *m_typemap;
  int                        m_features;
  Array                      m_soap_headers;
  int                        m_send_errors;

  static const StaticString  s_className;
};

///////////////////////////////////////////////////////////////////////////////

class SoapClient {
public:
  SoapClient();

  int                         m_soap_version;
  sdl                        *m_sdl;
  xmlCharEncodingHandlerPtr   m_encoding;
  encodeMap                  *m_typemap;
  Array                       m_classmap;
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
  int                         m_connection_timeout;
  int                         m_max_redirect;
  bool                        m_use11;
  String                      m_user_agent;
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

  static const StaticString   s_className;
};

///////////////////////////////////////////////////////////////////////////////

extern const StaticString
  s_enc_type, s_enc_value, s_enc_stype,
  s_enc_ns, s_enc_name, s_enc_namens;

class SoapVar {
public:
  static Class* getClass();

  static int64_t getEncType(ObjectData* obj) {
    auto enc_type = obj->o_get(s_enc_type, false);
    if (!enc_type.isInitialized()) {
      raise_error("Encoding: SoapVar has no 'enc_type' property");
      not_reached();
      assert(false);
    }
    return enc_type.toInt64();
  }

  static void setEncType(ObjectData* obj, int64_t t) {
    obj->o_set(s_enc_type, t);
  }

  static Variant getEncValue(ObjectData* obj) {
    return obj->o_get(s_enc_value, false);
  }

  static void setEncValue(ObjectData* obj, const Variant& val) {
    obj->o_set(s_enc_value, val);
  }

#define X(Name, str_name) \
  static void setEnc##Name(ObjectData* obj, const String& str) { \
    obj->o_set(s_enc_##str_name, str); \
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

  static Class*               s_class;
  static const StaticString   s_className;
};

///////////////////////////////////////////////////////////////////////////////

class SoapParam {
public:
  static Class* getClass();

  String                      m_name;
  String                      m_data;

  static Class*               s_class;
  static const StaticString   s_className;
};

///////////////////////////////////////////////////////////////////////////////

class SoapHeader {
public:
  static Class* getClass();

  String                      m_namespace;
  String                      m_name;
  Variant                     m_data;
  bool                        m_mustUnderstand;
  Variant                     m_actor;

  static Class*               s_class;
  static const StaticString   s_className;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_SOAP_H_
