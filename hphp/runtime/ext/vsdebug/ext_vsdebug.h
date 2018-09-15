/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_EXT_VSDEBUG_H_
#define incl_HPHP_EXT_VSDEBUG_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/vsdebug/transport.h"
#include "hphp/runtime/ext/vsdebug/fdtransport.h"
#include "hphp/runtime/ext/vsdebug/socket_transport.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"

namespace HPHP {
namespace VSDEBUG {

#define VSDEBUG_NAME "vsdebug"
#define VSDEBUG_VERSION "1.1"

struct VSDebugExtension final : Extension {
  VSDebugExtension() : Extension(VSDEBUG_NAME, VSDEBUG_VERSION) { }
  ~VSDebugExtension();

  void moduleLoad(const IniSetting::Map& ini, const Hdf hdf) override;
  void moduleInit() override;
  void moduleShutdown() override;
  void requestInit() override;
  void requestShutdown() override;
  bool moduleEnabled() const override { return m_enabled; }

  static Debugger* getDebugger() { return s_debugger; }
  static bool s_launchMode;

private:

  // The following members are set during module load and init and read only
  // after that. They can therefore be safely accessed lock-free.
  bool m_enabled {false};
  static Debugger* s_debugger;

  static constexpr int DefaultListenPort = 8999;
  static bool s_configEnabled;
  static std::string s_logFilePath;
  static int s_attachListenPort;
};

}
}

#endif // incl_HPHP_EXT_VSDEBUG_H_
