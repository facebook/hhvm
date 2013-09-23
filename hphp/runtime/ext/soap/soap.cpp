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

#include "hphp/runtime/ext/soap/soap.h"
#include "hphp/runtime/ext/soap/encoding.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

SoapData::SoapData() : m_cache(WSDL_CACHE_MEMORY), m_cache_ttl(86400) {
  for (int i = 0; s_defaultEncoding[i].type != END_KNOWN_TYPES; ++i){
    encodeStatic &e = s_defaultEncoding[i];

    encodePtr enc(new encode());
    enc->details.type = e.type;
    enc->details.type_str = e.type_str;
    enc->details.ns = e.ns;
    enc->to_zval = e.to_zval;
    enc->to_xml = e.to_xml;

    const encodeType &details = enc->details;
    if (!details.type_str.empty()) {
      string name = details.type_str;
      if (!details.ns.empty()) {
        name = string(details.ns) + ':' + name;
      }
      m_defEnc[name] = enc;
    }

    if (m_defEncIndex.find(details.type) == m_defEncIndex.end()) {
      m_defEncIndex[details.type] = enc;
    }
  }

  m_defEncNs[XSD_1999_NAMESPACE]     = XSD_NS_PREFIX;
  m_defEncNs[XSD_NAMESPACE]          = XSD_NS_PREFIX;
  m_defEncNs[XSI_NAMESPACE]          = XSI_NS_PREFIX;
  m_defEncNs[XML_NAMESPACE]          = XML_NS_PREFIX;
  m_defEncNs[SOAP_1_1_ENC_NAMESPACE] = SOAP_1_1_ENC_NS_PREFIX;
  m_defEncNs[SOAP_1_2_ENC_NAMESPACE] = SOAP_1_2_ENC_NS_PREFIX;
}

sdl *SoapData::get_sdl(const char *uri, long cache_wsdl,
                       HttpClient *http /* = NULL */) {
  sdlPtr sdl = get_sdl_impl(uri, cache_wsdl, http);
  if (sdl) {
    // holding it for the entire request life time, so soapserver and
    // soapclient can use sdl* without being deleted
    m_sdls.push_back(sdl);
    return sdl.get();
  }
  return NULL;
}

encodeMap *SoapData::register_typemap(encodeMapPtr typemap) {
  if (typemap) {
    // holding it for the entire request life time, so soapserver and
    // soapclient can use encodeMap* without being deleted
    m_typemaps.push_back(typemap);
    return typemap.get();
  }
  return NULL;
}

void SoapData::register_encoding(xmlCharEncodingHandlerPtr encoding) {
  if (encoding) {
    m_encodings.push_back(encoding);
  }
}

sdlPtr SoapData::get_sdl_impl(const char *uri, long cache_wsdl,
                              HttpClient *http) {
  if (cache_wsdl & WSDL_CACHE_MEMORY) {
    sdlCache::iterator iter = m_mem_cache.find(uri);
    if (iter != m_mem_cache.end()) {
      sdlCacheBucketPtr p = iter->second;
      if (p->time >= time(0) - m_cache_ttl) {
        return p->sdl;
      }
      /* in-memory cache entry is expired */
      m_mem_cache.erase(iter);
    }
  }

  const char *old = m_error_code;
  m_error_code = "WSDL";
  sdlPtr sdl = load_wsdl((char*)uri, http);
  m_error_code = old;

  if (sdl && (cache_wsdl & WSDL_CACHE_MEMORY)) {
    sdlCacheBucketPtr p(new sdlCacheBucket());
    p->sdl = sdl;
    p->time = time(0);
    m_mem_cache[uri] = p;
  }

  return sdl;
}

void SoapData::reset() {
  m_soap_version = SOAP_1_1;
  m_sdl = NULL;
  m_encoding = NULL;
  m_classmap.reset();
  m_typemap = NULL;
  m_features = 0;

  m_use_soap_error_handler = false;
  m_error_code = NULL;
  m_error_object.reset();

  m_cur_uniq_ns = 0;
  m_cur_uniq_ref = 0;
  m_ref_map.reset();

  m_sdls.clear();
  m_typemaps.clear();

  for (unsigned int i = 0; i < m_encodings.size(); i++) {
    xmlCharEncCloseFunc(m_encodings[i]);
  }
  m_encodings.clear();
}

IMPLEMENT_REQUEST_LOCAL(SoapData, s_soap_data);
///////////////////////////////////////////////////////////////////////////////

SoapException::SoapException(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  format((std::string("SOAP_ERROR: ") + fmt).c_str(), ap);
  va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////
}
