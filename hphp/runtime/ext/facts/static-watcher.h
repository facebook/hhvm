/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "hphp/runtime/ext/facts/watcher.h"

namespace HPHP {
namespace Facts {

/**
 * Return a Watcher which always returns the files originally passed in,
 * regardless of whether or not they've changed. It will also always claim these
 * files exist, even if they don't, and it will never hash the files.
 */
std::shared_ptr<Watcher> make_static_watcher(
    std::vector<std::filesystem::path> paths);

} // namespace Facts
} // namespace HPHP
