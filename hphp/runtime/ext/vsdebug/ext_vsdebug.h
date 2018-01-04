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

namespace HPHP {
namespace VSDEBUG {

#define VSDEBUG_NAME "vsdebug"
#define VSDEBUG_VERSION "1.0"

struct VSDebugExtension final : Extension {
  VSDebugExtension() : Extension(VSDEBUG_NAME, VSDEBUG_VERSION) { }
  ~VSDebugExtension();

  void moduleLoad(const IniSetting::Map& ini, Hdf hdf) override;
  void moduleInit() override;
  void requestInit() override;
  void requestShutdown() override;
  bool moduleEnabled() const override { return m_enabled; }

  static constexpr int DefaultListenPort = 8999;
  static bool s_configEnabled;
  static std::string s_logFilePath;
  static int s_attachListenPort;

private:
  bool m_enabled {false};
};

}
}

#endif // incl_HPHP_EXT_VSDEBUG_H_
