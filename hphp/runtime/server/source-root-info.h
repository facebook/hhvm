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

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/debugger/debugger_base.h"

#include <filesystem>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Transport;

namespace SourceRootInfo {

bool Init(Transport* transport);
bool Init(const std::string& user, const std::string& sandbox);

void ResetLogging();

Array SetServerVariables(Array server);
bool SandboxOn();
bool IsInitialized();

std::filesystem::path GetCurrentSourceRoot();
String RelativeToPhpRoot(const String& path);

Eval::DSandboxInfo GetSandboxInfo();

struct WithRoot {
  explicit WithRoot(Transport* transport) { Init(transport); }
  WithRoot(const std::string& user, const std::string& sandbox) {
    Init(user, sandbox);
  }
  WithRoot(WithRoot&&) = delete;
  WithRoot& operator=(WithRoot&&) = delete;

  ~WithRoot() { ResetLogging(); }
};

}}
