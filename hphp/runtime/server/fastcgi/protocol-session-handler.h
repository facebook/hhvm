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

#ifndef incl_HPHP_RUNTIME_SERVER_FASTCGI_SESSION_HANDLER_H_
#define incl_HPHP_RUNTIME_SERVER_FASTCGI_SESSION_HANDLER_H_

#include "hphp/util/base.h"
#include "folly/io/IOBuf.h"
#include <map>

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(ProtocolSessionHandler);
class ProtocolSessionHandler {
public:
  class Callback {
  public:
    virtual ~Callback() {}

    virtual void onStdOut(std::unique_ptr<folly::IOBuf> chain) = 0;
    virtual void onStdErr(std::unique_ptr<folly::IOBuf> chain) = 0;
    virtual void onComplete() = 0;
  };

  virtual ~ProtocolSessionHandler() {}

  virtual void onBody(std::unique_ptr<folly::IOBuf> chain) = 0;
  virtual void onBodyComplete() = 0;
  virtual void onHeader(std::unique_ptr<folly::IOBuf> key_chain,
                        std::unique_ptr<folly::IOBuf> value_chain) = 0;
  virtual void onHeadersComplete() = 0;

  void setCallback(Callback* callback) {
    m_callback = callback;
  }

protected:
  Callback* m_callback;
};

////////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_FASTCGI_HANDLER_H_

