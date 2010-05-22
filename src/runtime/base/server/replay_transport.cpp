/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/server/replay_transport.h>
#include <runtime/base/string_util.h>
#include <runtime/base/zend/zend_functions.h>
#include <runtime/base/zend/zend_string.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void ReplayTransport::recordInput(Transport* transport, const char *filename) {
  ASSERT(transport);

  Hdf hdf;
  hdf["get"] = (transport->getMethod() == GET);
  hdf["url"] = transport->getUrl();
  hdf["remote_host"] = transport->getRemoteHost();

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
  m_hdf.open(filename);
  replayInputImpl();
}

void ReplayTransport::replayInput(Hdf hdf) {
  m_hdf.assign(hdf);
  replayInputImpl();
}

void ReplayTransport::replayInputImpl() {
  String postData = StringUtil::UUDecode(m_hdf["post"].get(""));
  m_postData = string(postData.data(), postData.size());
  m_requestHeaders.clear();
  for (Hdf hdf = m_hdf["headers"].firstChild(); hdf.exists();
       hdf = hdf.next()) {
    m_requestHeaders[hdf["name"].get("")].push_back(hdf["value"].get(""));
  }
}

const char *ReplayTransport::getUrl() {
  return m_hdf["url"].get("");
}

const char *ReplayTransport::getRemoteHost() {
  return m_hdf["remote_host"].get("");
}

const void *ReplayTransport::getPostData(int &size) {
  size = m_postData.size();
  return m_postData.data();
}

Transport::Method ReplayTransport::getMethod() {
  return m_hdf["get"].getBool() ? GET : POST;
}

std::string ReplayTransport::getHeader(const char *name) {
  ASSERT(name);
  if (m_requestHeaders.find(name) != m_requestHeaders.end()) {
    ASSERT(!m_requestHeaders[name].empty());
    return m_requestHeaders[name][0];
  }
  return "";
}

void ReplayTransport::getHeaders(HeaderMap &headers) {
  headers = m_requestHeaders;
}

void ReplayTransport::addHeaderImpl(const char *name, const char *value) {
  ASSERT(name && value);
  m_responseHeaders[name].push_back(value);
}

void ReplayTransport::removeHeaderImpl(const char *name) {
  ASSERT(name);
  m_responseHeaders.erase(name);
}

void ReplayTransport::sendImpl(const void *data, int size, int code,
                               bool chunked) {
  m_code = code;

  m_response = "HTTP/1.1 ";
  m_response += boost::lexical_cast<string>(code);
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
