/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <memory>
#include "watchman/query/FileResult.h"
#include "watchman/query/QueryResult.h"
#include "watchman/saved_state/SavedStateInterface.h"

namespace watchman {

struct Query;
struct QueryContext;
class Root;

// Generator callback, used to plug in an alternate
// generator when used in triggers or subscriptions
using QueryGenerator = std::function<void(
    const Query* query,
    const std::shared_ptr<Root>& root,
    QueryContext* ctx)>;

} // namespace watchman

/**
 * Execute a query against the root.
 *
 * savedStateFactory allows testing this function without pulling in a wide
 * set of dependencies.
 */
watchman::QueryResult w_query_execute(
    const watchman::Query* query,
    const std::shared_ptr<watchman::Root>& root,
    watchman::QueryGenerator generator,
    watchman::SavedStateFactory savedStateFactory);

// Allows a generator to process a file node
// through the query engine
void w_query_process_file(
    const watchman::Query* query,
    watchman::QueryContext* ctx,
    std::unique_ptr<watchman::FileResult> file);

void time_generator(
    const watchman::Query* query,
    const std::shared_ptr<watchman::Root>& root,
    watchman::QueryContext* ctx);
