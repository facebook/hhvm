/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Hook interface for extensions to perform initialization and
 * shutdown sequences at request scope.
 */
struct RequestEventHandler {
  RequestEventHandler() = default;
  RequestEventHandler(const RequestEventHandler&) = delete;
  RequestEventHandler& operator=(const RequestEventHandler&) = delete;

  virtual void requestInit() = 0;
  virtual void requestShutdown() = 0;

  /*
   * Priority of request shutdown call. Lower priority values are called
   * earlier than higher priority values. Since priorities are only meaningful
   * due to how they relate to each other, it's important to consider existing
   * overrides when overriding this (add your handler here if you do):
   */
  virtual int priority() const { return 0; }

  void setInited(bool inited) { m_inited = inited; }
  bool getInited() const { return m_inited; }

protected:
  // Never delete these polymorphically.
  ~RequestEventHandler() {}

private:
  bool m_inited = false;
};

//////////////////////////////////////////////////////////////////////

}

