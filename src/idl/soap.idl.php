<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

p(
<<<END
#include <system/gen/php/classes/exception.h>
#include <runtime/ext/soap/soap.h>
END
);

f('use_soap_error_handler', Boolean,
  array('handler' => array(Boolean, 'true')));
f('is_soap_fault', Boolean,
  array('fault' => Variant));

c('SoapServer', null, array(),
  array(
    m(PublicMethod, "__construct", null,
      array("wsdl" => Variant,
            "options" => array(VariantMap, "null_array"))),
    m(PublicMethod, "setClass", null,
      array("name" => String),
      VariableArguments),
    m(PublicMethod, "setObject", null,
      array("obj" => Object)),
    m(PublicMethod, "addFunction", null,
      array("func" => Variant)),
    m(PublicMethod, "getFunctions", Variant),
    m(PublicMethod, "handle", null,
      array("request" => array(String, "null_string"))),
    m(PublicMethod, "setPersistence", null,
      array("mode" => Int64)),
    m(PublicMethod, "fault", null,
      array("code" => Variant,
            "fault" => String,
            "actor" => array(String, "null_string"),
            "detail" => array(Variant, "null"),
            "name" => array(String, "null_string"))),
    m(PublicMethod, "addSoapHeader", null,
      array("fault" => Object)),
  ),
  array(),
  "\n  public: int                        m_type;".
  "\n  public: soapFunctions              m_soap_functions;".
  "\n  public: soapClass                  m_soap_class;".
  "\n  public: Object                     m_soap_object;".

  "\n  public: String                     m_actor;".
  "\n  public: String                     m_uri;".

  "\n  public: int                        m_version;".
  "\n  public: sdl                       *m_sdl;".
  "\n  public: xmlCharEncodingHandlerPtr  m_encoding;".
  "\n  public: Array                      m_classmap;".
  "\n  public: encodeMap                 *m_typemap;".
  "\n  public: int                        m_features;".

  "\n  public: Array                      m_soap_headers;".
  "\n  public: int                        m_send_errors;".

  ""
 );

c('SoapClient', null, array(),
  array(
    m(PublicMethod, "__construct", null,
      array("wsdl" => Variant,
            "options" => array(VariantMap, "null_array"))),
    m(PublicMethod, "__call", Variant,
      array("name" => Variant,
            "args" => Variant)),
    m(PublicMethod, "__soapCall", Variant,
      array("name" => String,
            "args" => VariantMap,
            "options" => array(VariantMap, "null_array"),
            "input_headers" => array(Variant, "null"),
            "output_headers" => array(Variant | Reference, "null"))),
    m(PublicMethod, "__getLastRequest", Variant),
    m(PublicMethod, "__getLastResponse", Variant),
    m(PublicMethod, "__getLastRequestHeaders", Variant),
    m(PublicMethod, "__getLastResponseHeaders", Variant),
    m(PublicMethod, "__getFunctions", Variant),
    m(PublicMethod, "__getTypes", Variant),
    m(PublicMethod, "__doRequest", Variant,
      array("buf" => String,
            "location" => String,
            "action" => String,
            "version" => Int64,
            "oneway" => array(Boolean, "false"))),
    m(PublicMethod, "__setCookie", Variant,
      array("name" => String,
            "value" => array(String, "null_string"))),
    m(PublicMethod, "__setLocation", Variant,
      array("new_location" => array(String, "null_string"))),
    m(PublicMethod, "__setSoapHeaders", Boolean,
      array("headers" => array(Variant, "null"))),
  ),
  array(),
  "\n  public: int                         m_soap_version;".
  "\n  public: sdl                        *m_sdl;".
  "\n  public: xmlCharEncodingHandlerPtr   m_encoding;".
  "\n  public: encodeMap                  *m_typemap;".
  "\n  public: Array                       m_classmap;".
  "\n  public: int                         m_features;".

  "\n  public: String                      m_uri;".
  "\n  public: String                      m_location;".
  "\n  public: int                         m_style;".
  "\n  public: int                         m_use;".

  "\n  public: String                      m_login;".
  "\n  public: String                      m_password;".
  "\n  public: int                         m_authentication;".
  "\n  public: bool                        m_digest;".

  "\n  public: String                      m_proxy_host;".
  "\n  public: int                         m_proxy_port;".
  "\n  public: String                      m_proxy_login;".
  "\n  public: String                      m_proxy_password;".

  "\n  public: int                         m_connection_timeout;".
  "\n  public: int                         m_max_redirect;".
  "\n  public: bool                        m_use11;".
  "\n  public: String                      m_user_agent;".
  "\n  public: bool                        m_compression;".

  "\n  public: Variant                     m_default_headers;".
  "\n  public: Variant                     m_cookies;".

  "\n  public: bool                        m_exceptions;".
  "\n  public: Variant                     m_soap_fault;".

  "\n  public: bool                        m_trace;".
  "\n  public: Variant                     m_last_request;".
  "\n  public: Variant                     m_last_response;".
  "\n  public: Variant                     m_last_request_headers;".
  "\n  public: Variant                     m_last_response_headers;".

  "\n"
 );

c('SoapVar', null, array(),
  array(
    m(PublicMethod, '__construct', null,
      array("data" => Variant,
            "type" => Variant,
            "type_name" => array(String, "null_string"),
            "type_namespace" => array(String, "null_string"),
            "node_name" => array(String, "null_string"),
            "node_namespace" => array(String, "null_string"),
           )),
  ),
  array(),
  "\n  public: Variant m_value;".
  "\n  public: int64   m_type;".
  "\n  public: String  m_stype;".
  "\n  public: String  m_ns;".
  "\n  public: String  m_name;".
  "\n  public: String  m_namens;"
 );

c('SoapFault', 'exception', array(),
  array(
    m(PublicMethod, '__construct', null,
      array("code" => Variant,
            "message" => String,
            "actor" => array(String, "null_string"),
            "detail" => array(Variant, "null"),
            "name" => array(String, "null_string"),
            "header" => array(Variant, "null"))),
    m(PublicMethod, '__toString', String),
  ),
  array(),
  "\n  public: String  m_faultstring;".
  "\n  public: String  m_faultcode;".
  "\n  public: String  m_faultcodens;".
  "\n  public: String  m_faultactor;".
  "\n  public: Variant m_detail;".
  "\n  public: String  m_name;".
  "\n  public: Variant m_headerfault;".
  "\n  private: int64  _dummy;  // HACK: this class must be at least".
  "\n                           // 176 bytes or else it causes a".
  "\n                           // double-free error during shutdown".
  "\n                           // when running 'make slow_tests'"
 );

c('SoapParam', null, array(),
  array(
    m(PublicMethod, '__construct', null,
      array("data" => Variant,
            "name" => String)),
  ),
  array(),
  "\n  public: String  m_name;".
  "\n  public: String  m_data;"
 );

c('SoapHeader', null, array(),
  array(
    m(PublicMethod, '__construct', null,
      array("ns" => String,
            "name" => String,
            "data" => array(Variant, "null"),
            "mustUnderstand" => array(Boolean, "false"),
            "actor" => array(Variant, "null"))),
  ),
  array(),
  "\n  public: String  m_namespace;".
  "\n  public: String  m_name;".
  "\n  public: Variant m_data;".
  "\n  public: bool    m_mustUnderstand;".
  "\n  public: Variant m_actor;"
 );
