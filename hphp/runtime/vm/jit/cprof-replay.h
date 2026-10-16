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

#pragma once

#include <string>

#include <folly/File.h>

namespace HPHP::jit::cprof {

/*
 * Hold an exclusive directory-level replay lease and persist an in-progress
 * marker before reading checkpoints. Failure or lock contention leaves
 * the guard inactive, so callers must skip replay.
 *
 * An abandoned marker discards all regular checkpoint files on the next startup
 * and skips replay for that startup. Cleanup failures leave the marker set.
 * Destruction releases the lease without clearing the marker; normal completion
 * must clear it explicitly. Checkpoint writes never reset the marker.
 * Never unlink or replace the lock file while a process may hold its inode.
 */
struct ContProfReplayGuard {
  explicit ContProfReplayGuard(const std::string& directory) noexcept;

  ContProfReplayGuard(const ContProfReplayGuard&) = delete;
  ContProfReplayGuard& operator=(const ContProfReplayGuard&) = delete;

  bool active() const { return bool(m_file); }

  // Call on normal exit, including when no profiles were imported.
  // The exclusive lease remains held until destruction.
  bool complete() noexcept;

private:
  folly::File m_file;
};

}
