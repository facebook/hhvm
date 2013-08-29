/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_REPLAY_TRANSPORT_H_
#define incl_HPHP_REPLAY_TRANSPORT_H_

#include "hphp/runtime/server/transport.h"
#include "hphp/util/hdf.h"
#include "hphp/runtime/base/complex-types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * This transport will capture what's in another transport's requests, and
 * record it in a file that later this same class can replay. Written for
 * debugging HTTP requests that cause problems that are hard to debug on live
 * servers.
 */
class ReplayTransport : public Transport {
public:
  ReplayTransport() : m_code(0) {}

  void recordInput(Transport* transport, const char *filename);
  void replayInput(const char *filename);
  void replayInput(Hdf hdf);

  /**
   * Implementing Transport...
   */
  virtual const char *getUrl();
  virtual const char *getRemoteHost();
  virtual uint16_t getRemotePort();
  virtual const void *getPostData(int &size);
  virtual Method getMethod();
  virtual std::string getHeader(const char *name);
  virtual void getHeaders(HeaderMap &headers);
  virtual void addHeaderImpl(const char *name, const char *value);
  virtual void removeHeaderImpl(const char *name);
  virtual void sendImpl(const void *data, int size, int code, bool chunked);

  int getResponseCode() const { return m_code;}
  const std::string &getResponse() const { return m_response;}

private:
  Hdf m_hdf;
  std::string m_postData;
  HeaderMap m_requestHeaders;
  HeaderMap m_responseHeaders;

  int m_code;
  std::string m_response;

  void replayInputImpl();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_REPLAY_TRANSPORT_H_
