/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/server/replay-transport.h"

#include <boost/lexical_cast.hpp>

#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/util/process.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void ReplayTransport::recordInput(Transport* transport, const char *filename) {
  assert(transport);

  Hdf hdf;

  char buf[32];
  snprintf(buf, sizeof(buf), "%u", Process::GetProcessId());
  hdf["pid"] = std::string(buf);
  snprintf(buf, sizeof(buf), "%" PRIx64, (int64_t)Process::GetThreadId());
  hdf["tid"] = std::string(buf);
  snprintf(buf, sizeof(buf), "%u", Process::GetThreadPid());
  hdf["tpid"] = std::string(buf);

  hdf["cmd"] = static_cast<int>(transport->getMethod());
  hdf["url"] = transport->getUrl();
  hdf["remote_host"] = transport->getRemoteHost();
  hdf["remote_port"] = transport->getRemotePort();

  transport->getHeaders(m_requestHeaders);
  int index = 0;
  for (HeaderMap::const_iterator iter = m_requestHeaders.begin();
       iter != m_requestHeaders.end(); ++iter) {
    for (unsigned int i = 0; i < iter->second.size(); i++) {
      Hdf header = hdf["headers"][index++];
      header["name"] = iter->first;
      header["value"] = iter->second[i];
    }
  }

  int size;
  const void *data = transport->getPostData(size);
  if (size) {
    int len;
    char *encoded = string_uuencode((const char *)data, size, len);
    hdf["post"] = encoded;
    free(encoded);
  } else {
    hdf["post"] = "";
  }

  hdf.write(filename);
}

void ReplayTransport::replayInput(const char *filename) {
  replayInputImpl(Hdf(filename));
}

void ReplayTransport::replayInput(Hdf hdf) {
  replayInputImpl(hdf);
}

void ReplayTransport::replayInputImpl(const Hdf &hdf) {
  // TODO support setting these from ini
  IniSetting::Map ini = IniSetting::Map::object;

  std::string postData;
  RuntimeOption::Bind(postData, ini, hdf, "post");
  auto postDataString = StringUtil::UUDecode(String(postData));
  m_postData = std::string(postDataString.data(), postDataString.size());
  m_requestHeaders.clear();
  for (Hdf hdf = hdf["headers"].firstChild(); hdf.exists();
       hdf = hdf.next()) {
    m_requestHeaders[hdf["name"].get("")].push_back(hdf["value"].get(""));
  }

  RuntimeOption::Bind(m_url, ini, hdf, "url");
  RuntimeOption::Bind(m_remoteHost, ini, hdf, "remote_host");
  RuntimeOption::Bind(m_remotePort, ini, hdf, "remote_port");
  RuntimeOption::Bind(m_method, ini, hdf, "cmd");
}

const char *ReplayTransport::getUrl() {
  return m_url.data();
}

const char *ReplayTransport::getRemoteHost() {
  return m_remoteHost.data();
}
uint16_t ReplayTransport::getRemotePort() {
  return m_remotePort;
}

const void *ReplayTransport::getPostData(int &size) {
  size = m_postData.size();
  return m_postData.data();
}

Transport::Method ReplayTransport::getMethod() {
  return (Transport::Method)m_method;
}

std::string ReplayTransport::getHeader(const char *name) {
  assert(name);
  if (m_requestHeaders.find(name) != m_requestHeaders.end()) {
    assert(!m_requestHeaders[name].empty());
    return m_requestHeaders[name][0];
  }
  return "";
}

void ReplayTransport::getHeaders(HeaderMap &headers) {
  headers = m_requestHeaders;
}

void ReplayTransport::addHeaderImpl(const char *name, const char *value) {
  assert(name && value);
  m_responseHeaders[name].push_back(value);
}

void ReplayTransport::removeHeaderImpl(const char *name) {
  assert(name);
  m_responseHeaders.erase(name);
}

void ReplayTransport::sendImpl(const void *data, int size, int code,
                               bool chunked) {
  m_code = code;

  m_response = "HTTP/1.1 ";
  m_response += boost::lexical_cast<std::string>(code);
  m_response += " ";
  m_response += (m_code == 200 ? "OK" : "Internal Server Error");
  m_response += "\r\n";

  for (HeaderMap::const_iterator iter = m_responseHeaders.begin();
       iter != m_responseHeaders.end(); ++iter) {
    for (unsigned int i = 0; i < iter->second.size(); i++) {
      m_response += iter->first;
      m_response += ": ";
      m_response += iter->second[i];
      m_response += "\r\n";
    }
  }

  m_response += "\r\n";
  m_response.append((const char *)data, size);
  m_response += "\r\n";
}

///////////////////////////////////////////////////////////////////////////////
}
