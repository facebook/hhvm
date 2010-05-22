/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef PHP_SOAP_H
#define PHP_SOAP_H

#include <runtime/base/base_includes.h>
#include <runtime/ext/soap/sdl.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/util/exceptions.h>
#include <util/lock.h>

///////////////////////////////////////////////////////////////////////////////
// defines

#define SOAP_CLASS 1
#define SOAP_FUNCTIONS 2
#define SOAP_OBJECT 3
#define SOAP_FUNCTIONS_ALL 999

#define SOAP_MAP_FUNCTION 1
#define SOAP_MAP_CLASS 2

#define SOAP_PERSISTENCE_SESSION 1
#define SOAP_PERSISTENCE_REQUEST 2

#define SOAP_1_1 1
#define SOAP_1_2 2

#define SOAP_ACTOR_NEXT             1
#define SOAP_ACTOR_NONE             2
#define SOAP_ACTOR_UNLIMATERECEIVER 3

#define SOAP_1_1_ACTOR_NEXT                     \
  "http://schemas.xmlsoap.org/soap/actor/next"
#define SOAP_1_2_ACTOR_NEXT                             \
  "http://www.w3.org/2003/05/soap-envelope/role/next"
#define SOAP_1_2_ACTOR_NONE                             \
  "http://www.w3.org/2003/05/soap-envelope/role/none"
#define SOAP_1_2_ACTOR_UNLIMATERECEIVER                                 \
  "http://www.w3.org/2003/05/soap-envelope/role/ultimateReceiver"

#define SOAP_COMPRESSION_ACCEPT  0x20
#define SOAP_COMPRESSION_GZIP    0x00
#define SOAP_COMPRESSION_DEFLATE 0x10

#define SOAP_AUTHENTICATION_BASIC   0
#define SOAP_AUTHENTICATION_DIGEST  1

#define SOAP_SINGLE_ELEMENT_ARRAYS  (1<<0)
#define SOAP_WAIT_ONE_WAY_CALLS     (1<<1)
#define SOAP_USE_XSI_ARRAY_TYPE     (1<<2)

#define WSDL_CACHE_NONE     0x0
#define WSDL_CACHE_DISK     0x1
#define WSDL_CACHE_MEMORY   0x2
#define WSDL_CACHE_BOTH     0x3

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class SoapData : public RequestEventHandler {
private:
  // SDL cache
  DECLARE_BOOST_TYPES(sdlCacheBucket);
  struct sdlCacheBucket {
    sdlPtr sdl;
    time_t time;
  };
  typedef StringTosdlCacheBucketPtrMap sdlCache;

public:
  int64 m_cache;

private:
  int64 m_cache_ttl;
  sdlCache m_mem_cache; // URL => sdl

public:
  sdl *get_sdl(const char *uri, long cache_wsdl);
  encodeMap *register_typemap(encodeMapPtr typemap);
  void register_encoding(xmlCharEncodingHandlerPtr encoding);

public:
  SoapData();

  // globals that live across requests
  encodeMap m_defEnc;   // name => encode
  std::map<int, encodePtr> m_defEncIndex; // type => encode
  std::map<std::string, std::string> m_defEncNs; // namespaces => prefixes

public:
  // request scope globals to avoid passing them between functions
  int m_soap_version;
  sdl *m_sdl;
  xmlCharEncodingHandlerPtr m_encoding;
  Array m_classmap; // typename => class name
  encodeMap *m_typemap;  // typename => encode
  int m_features;

  // error handling
  bool m_use_soap_error_handler;
  const char *m_error_code;
  Object m_error_object;

  // misc
  int m_cur_uniq_ns;
  int m_cur_uniq_ref;
  Array m_ref_map; // reference handling

public:
  virtual void requestInit() { reset();}
  virtual void requestShutdown() { reset();}

private:
  sdlPtrVec m_sdls;
  std::vector<encodeMapPtr> m_typemaps;
  std::vector<xmlCharEncodingHandlerPtr> m_encodings;

  sdlPtr get_sdl_impl(const char *uri, long cache_wsdl);
  void reset();
};

#define USE_SOAP_GLOBAL  SoapData *__sg__ = s_soap_data.get();
#define SOAP_GLOBAL(name) __sg__->m_##name
DECLARE_EXTERN_REQUEST_LOCAL(SoapData, s_soap_data);

///////////////////////////////////////////////////////////////////////////////
// types used by SoapServer

struct soapFunctions {
  Array ft;
  Array ftOriginal;
  bool functions_all;
};

struct soapClass {
  String name;
  Array argv;
  int persistance;
};

class soapHeader : public ResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(soapHeader);

  // overriding ResourceData
  virtual const char *o_getClassName() const { return "soapHeader";}

  sdlFunction                      *function;
  String                            function_name;
  bool                              mustUnderstand;
  Array                             parameters;
  Variant                           retval;
  sdlSoapBindingFunctionHeader     *hdr;
};

///////////////////////////////////////////////////////////////////////////////

class SoapException : public ExtendedException {
public:
  SoapException(const char *fmt, ...);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
