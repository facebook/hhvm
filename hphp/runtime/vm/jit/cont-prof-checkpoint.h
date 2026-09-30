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

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <folly/Range.h>

#include "hphp/runtime/vm/jit/cont-prof-record.h"

namespace HPHP::jit {

/* Whether the process-wide checkpoint writer is accepting profile data. */
bool contProfActive();

/* Start and stop the process-wide checkpoint writer. */
void startContProfCheckpointWriter();
void stopContProfCheckpointWriter();

/* Encode key-ordered records into a checkpoint blob. */
std::optional<std::vector<uint8_t>> serializeContProfCheckpoint(
  const std::vector<ContProfProfileRecord>& records);

/* Decode a checkpoint blob, validating untrusted input. */
std::optional<std::vector<ContProfProfileRecord>>
deserializeContProfCheckpoint(folly::ByteRange);

/* Atomically replace path with a checkpoint of records. */
bool writeContProfCheckpointFile(
  const std::string& path,
  const std::vector<ContProfProfileRecord>& records);

/* Read the checkpoint at path; nullopt if missing or malformed. */
std::optional<std::vector<ContProfProfileRecord>>
readContProfCheckpointFile(const std::string& path);

}
