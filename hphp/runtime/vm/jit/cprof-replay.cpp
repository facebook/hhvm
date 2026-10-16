/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/vm/jit/cprof-replay.h"

#include <exception>
#include <fcntl.h>
#include <filesystem>
#include <sys/stat.h>
#include <utility>

#include <folly/FileUtil.h>

#include "hphp/runtime/vm/jit/cprof-checkpoint.h"
#include "hphp/util/logger.h"

namespace HPHP::jit::cprof {

namespace {

bool clearReplayMarker(int fd) noexcept {
  if (folly::ftruncateNoInt(fd, 0) != 0 || folly::fsyncNoInt(fd) != 0) {
    Logger::Warning("cont-prof replay could not clear its replay marker");
    return false;
  }
  return true;
}

}

ContProfReplayGuard::ContProfReplayGuard(
  const std::string& directory
) noexcept {
  try {
    if (directory.empty()) {
      Logger::Warning("cont-prof replay skipped: empty checkpoint directory");
      return;
    }

    auto const path = directory + "/cont-prof-replay.lock";
    folly::File file{
      path, O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW, 0600
    };
    if (!file.try_lock()) {
      Logger::Warning(
        "cont-prof replay skipped: another replay holds %s", path.c_str()
      );
      return;
    }

    struct stat status{};
    if (::fstat(file.fd(), &status) != 0 || !S_ISREG(status.st_mode)) {
      Logger::Warning(
        "cont-prof replay skipped: cannot inspect replay lock %s",
        path.c_str()
      );
      return;
    }
    if (status.st_size != 0) {
      Logger::Warning(
        "cont-prof replay skipped: previous replay did not complete. "
        "Discarding checkpoints in %s",
        directory.c_str()
      );
      for (auto const& entry : std::filesystem::directory_iterator{directory}) {
        if (!isContProfCheckpointFilename(entry.path().filename().native())) {
          continue;
        }
        if (!std::filesystem::is_regular_file(entry.symlink_status())) continue;
        std::filesystem::remove(entry.path());
      }
      clearReplayMarker(file.fd());
      return;
    }

    if (folly::ftruncateNoInt(file.fd(), 1) != 0 ||
        folly::fsyncNoInt(file.fd()) != 0) {
      Logger::Warning(
        "cont-prof replay skipped: cannot persist replay marker %s",
        path.c_str()
      );
      return;
    }

    m_file = std::move(file);
  } catch (const std::exception& exception) {
    Logger::Warning("cont-prof replay skipped: %s", exception.what());
  } catch (...) {
    Logger::Warning("cont-prof replay skipped: cannot acquire replay guard");
  }
}

bool ContProfReplayGuard::complete() noexcept {
  return active() && clearReplayMarker(m_file.fd());
}

}
