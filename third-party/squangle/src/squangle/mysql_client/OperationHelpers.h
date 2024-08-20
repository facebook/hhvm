/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/FetchOperation.h"

namespace facebook::common::mysql_client {

RowBlock makeRowBlockFromStream(
    std::shared_ptr<RowFields> row_fields,
    RowStream* row_stream);

} // namespace facebook::common::mysql_client
