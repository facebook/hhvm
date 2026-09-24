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

#include <filesystem>
#include <string>

#include "hphp/util/blob-encoder.h"

namespace HPHP {

struct RepoOptions;
struct RepoOptionsFlags;

struct UnitEmitterAttributes {
  bool strictPackage : 1 {false};
  bool raiseDynamicClassLoadError : 1 {false};

  static UnitEmitterAttributes defaults();
  static UnitEmitterAttributes forAbsolutePath(
    const std::filesystem::path&,
    const RepoOptions&
  );
  static UnitEmitterAttributes forRepoRelativePath(
    const std::filesystem::path&,
    const RepoOptionsFlags&
  );
  std::string mangle() const;

  template <typename SerDe> void serde(SerDe& sd) {
    SERDE_BITFIELD(strictPackage, sd);
    SERDE_BITFIELD(raiseDynamicClassLoadError, sd);
  }
};

static_assert(sizeof(UnitEmitterAttributes) == 1);

}
