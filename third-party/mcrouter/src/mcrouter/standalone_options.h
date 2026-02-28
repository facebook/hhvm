/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <sys/socket.h>

#include "mcrouter/options.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

/*
** Observation of mcrouter in production indicates that
** that delta(RSS) / delta(outstanding reqs) is just under
** 3K.
*/
#define OUTSTANDING_REQ_BYTES (3 * 1024)
#define DEFAULT_MAX_CLIENT_OUTSTANDING_REQS \
  (uint32_t)((1024 * 1024 * 100) / OUTSTANDING_REQ_BYTES)

#define OPTIONS_FILE "mcrouter/standalone_options_list.h"
#define OPTIONS_NAME McrouterStandaloneOptions
#include "mcrouter/options-template.h"

#undef OPTIONS_FILE
#undef OPTIONS_NAME

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
