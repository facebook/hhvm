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

#include "hphp/runtime/base/base-includes.h"
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
  bool                        m_compression;
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

class SoapVar {
public:
  static Class* getClass();

  Variant                     m_value;
  int64_t                     m_type;
  String                      m_stype;
  String                      m_ns;
  String                      m_name;
  String                      m_namens;

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
