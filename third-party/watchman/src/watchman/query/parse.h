/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <vector>
#include "watchman/Clock.h"
#include "watchman/fs/FileSystem.h"

namespace watchman {

struct Query;
struct QueryFieldRenderer;
class QueryFieldList;
class Root;

constexpr inline std::chrono::milliseconds kDefaultQuerySyncTimeout{60000};

std::shared_ptr<watchman::Query> parseQuery(
    const std::shared_ptr<watchman::Root>& root,
    const json_ref& query);

// parse the old style since and find queries
std::shared_ptr<watchman::Query> parseQueryLegacy(
    const std::shared_ptr<watchman::Root>& root,
    const json_ref& args,
    int start,
    uint32_t* next_arg,
    const char* clockspec,
    json_ref* expr_p);

void w_query_legacy_field_list(watchman::QueryFieldList* flist);

void parse_field_list(
    const std::optional<json_ref>& field_list,
    watchman::QueryFieldList* selected);
json_ref field_list_to_json_name_array(
    const watchman::QueryFieldList& fieldList);

void parse_suffixes(watchman::Query* res, const json_ref& query);
void parse_globs(watchman::Query* res, const json_ref& query);

} // namespace watchman
