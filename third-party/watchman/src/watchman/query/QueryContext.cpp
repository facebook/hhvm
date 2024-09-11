/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/query/QueryContext.h"

#include "folly/stop_watch.h"

#include "watchman/query/Query.h"
#include "watchman/query/eval.h"
#include "watchman/query/parse.h"
#include "watchman/root/Root.h"
#include "watchman/watchman_file.h"

using namespace watchman;

namespace {

constexpr size_t kMaximumRenderBatchSize = 1024;

std::optional<json_ref> file_result_to_json(
    const QueryFieldList& fieldList,
    const std::unique_ptr<FileResult>& file,
    const QueryContext* ctx) {
  if (fieldList.size() == 1) {
    return fieldList.front()->make(file.get(), ctx);
  }
  std::unordered_map<w_string, json_ref> value;
  value.reserve(fieldList.size());

  for (auto& f : fieldList) {
    auto ele = f->make(file.get(), ctx);
    if (!ele.has_value()) {
      // Need data to be loaded
      return std::nullopt;
    }
    value.insert_or_assign(f->name, std::move(ele.value()));
  }
  return json_object(std::move(value));
}

} // namespace

void QueryContext::resetWholeName() {
  wholename_.reset();
}

const w_string& QueryContext::getWholeName() {
  if (!wholename_) {
    wholename_ = computeWholeName(file.get());
  }
  return *wholename_;
}

w_string QueryContext::computeWholeName(FileResult* file) const {
  uint32_t name_start;

  if (query->relative_root) {
    // At this point every path should start with the relative root, so this is
    // legal
    name_start = query->relative_root->size() + 1;
  } else {
    name_start = root->root_path.size() + 1;
  }

  // Record the name relative to the root
  auto parent = file->dirName();
  if (name_start > parent.size()) {
    return file->baseName().asWString();
  }
  parent.advance(name_start);
  return w_string::build(parent, "/", file->baseName());
}

bool QueryContext::dirMatchesRelativeRoot(w_string_piece fullDirectoryPath) {
  if (!query->relative_root) {
    return true;
  }

  // "matches relative root" here can be either an exact match for
  // the relative root, or some path below it, so we compare against
  // both.  relative_root_slash is a precomputed version of relative_root
  // with the trailing slash to make this comparison very slightly cheaper
  // and less awkward to express in code.
  return fullDirectoryPath == query->relative_root ||
      fullDirectoryPath.startsWith(query->relative_root_slash);
}

bool QueryContext::fileMatchesRelativeRoot(w_string_piece fullFilePath) {
  // dirName() scans the string contents; avoid it with this cheap test
  if (!query->relative_root) {
    return true;
  }

  return dirMatchesRelativeRoot(fullFilePath.dirName());
}

bool QueryContext::fileMatchesRelativeRoot(const watchman_file* f) {
  // getFullPath() allocates memory; avoid it with this cheap test
  if (!query->relative_root) {
    return true;
  }

  return dirMatchesRelativeRoot(f->parent->getFullPath());
}

QueryContext::QueryContext(
    const Query* q,
    const std::shared_ptr<Root>& root,
    bool disableFreshInstance)
    : created(std::chrono::steady_clock::now()),
      query(q),
      root(root),
      disableFreshInstance{disableFreshInstance} {}

void QueryContext::addToEvalBatch(std::unique_ptr<FileResult>&& file) {
  evalBatch_.emplace_back(std::move(file));

  // Find a balance between local memory usage, latency in fetching
  // and the cost of fetching the data needed to re-evaluate this batch.
  // TODO: maybe allow passing this number in via the query?
  if (evalBatch_.size() >= 20480) {
    fetchEvalBatchNow();
  }
}

void QueryContext::fetchEvalBatchNow() {
  if (evalBatch_.empty()) {
    return;
  }
  folly::stop_watch<std::chrono::microseconds> timer;
  evalBatch_.front()->batchFetchProperties(evalBatch_);
  edenFilePropertiesDurationUs.fetch_add(timer.elapsed().count());

  auto toProcess = std::move(evalBatch_);

  for (auto& file : toProcess) {
    w_query_process_file(query, this, std::move(file));
  }

  w_assert(evalBatch_.empty(), "should have no files that NeedDataLoad");
}

RenderResult QueryContext::renderResults() {
  std::optional<json_ref> templ;
  if (query->fieldList.size() > 1) {
    // build a template for the serializer
    templ = field_list_to_json_name_array(query->fieldList);
  }
  return RenderResult{std::move(resultsArray), std::move(templ)};
}

void QueryContext::maybeRender(std::unique_ptr<FileResult>&& file) {
  auto maybeRendered = file_result_to_json(query->fieldList, file, this);
  if (maybeRendered.has_value()) {
    resultsArray.push_back(std::move(maybeRendered.value()));
    return;
  }

  addToRenderBatch(std::move(file));
}

void QueryContext::addToRenderBatch(std::unique_ptr<FileResult>&& file) {
  renderBatch_.emplace_back(std::move(file));
  // TODO: maybe allow passing this number in via the query?
  if (renderBatch_.size() >= kMaximumRenderBatchSize) {
    fetchRenderBatchNow();
  }
}

bool QueryContext::fetchRenderBatchNow() {
  if (renderBatch_.empty()) {
    return true;
  }

  folly::stop_watch<std::chrono::microseconds> timer;
  renderBatch_.front()->batchFetchProperties(renderBatch_);
  edenFilePropertiesDurationUs.fetch_add(timer.elapsed().count());

  auto toProcess = std::move(renderBatch_);

  for (auto& file : toProcess) {
    auto maybeRendered = file_result_to_json(query->fieldList, file, this);
    if (maybeRendered.has_value()) {
      resultsArray.push_back(std::move(maybeRendered.value()));
    } else {
      renderBatch_.emplace_back(std::move(file));
    }
  }

  return renderBatch_.empty();
}
