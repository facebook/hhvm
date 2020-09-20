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

#pragma once

namespace HPHP {
namespace VSDEBUG {

// Typedefs for settings particular to the current connected client.
enum PathFormat {
  Path,
  URI
};

struct ClientPreferences {
  bool linesStartAt1;
  bool columnsStartAt1;
  bool supportsVariableType;
  bool supportsVariablePaging;
  bool supportsRunInTerminalRequest;
  PathFormat pathFormat;
};


}
}

