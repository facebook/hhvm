/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/server/InternalPriorityRequestPile.h>

namespace apache::thrift {

InternalPriorityRequestPile::InternalPriorityRequestPile(Options opts)
    : RequestPileBase(opts.name), loPriPile_{opts}, highPriPile_{opts} {}

std::optional<ServerRequestRejection> InternalPriorityRequestPile::enqueue(
    ServerRequest&& request) {
  if (detail::ServerRequestHelper::internalPriority(request) ==
      folly::Executor::LO_PRI) {
    return loPriPile_.enqueue(std::move(request));
  } else {
    return highPriPile_.enqueue(std::move(request));
  }
}

std::optional<ServerRequest> InternalPriorityRequestPile::dequeue() {
  if (auto req = highPriPile_.dequeue()) {
    return req;
  }
  return loPriPile_.dequeue();
}

uint64_t InternalPriorityRequestPile::requestCount() const {
  return loPriPile_.requestCount() + highPriPile_.requestCount();
}

std::string InternalPriorityRequestPile::describe() const {
  std::string result = fmt::format(
      "InternalPriorityRequestPile LO_PRI:{} HIGH_PRI:{}\n",
      loPriPile_.describe(),
      highPriPile_.describe());
  return result;
}

serverdbginfo::RequestPileDbgInfo InternalPriorityRequestPile::getDbgInfo()
    const {
  serverdbginfo::RequestPileDbgInfo info;
  info.name() = folly::demangle(typeid(*this));

  auto loPriPileInfo = loPriPile_.getDbgInfo();
  auto hiPriPileInfo = loPriPile_.getDbgInfo();

  info.prioritiesCount() =
      *loPriPileInfo.prioritiesCount() + *hiPriPileInfo.prioritiesCount();

  info.bucketsPerPriority()->insert(
      info.bucketsPerPriority()->end(),
      (*loPriPileInfo.bucketsPerPriority()).begin(),
      (*loPriPileInfo.bucketsPerPriority()).end());

  info.bucketsPerPriority()->insert(
      info.bucketsPerPriority()->end(),
      (*hiPriPileInfo.bucketsPerPriority()).begin(),
      (*hiPriPileInfo.bucketsPerPriority()).end());

  info.perBucketRequestLimit() = std::max(
      *loPriPileInfo.perBucketRequestLimit(),
      *hiPriPileInfo.perBucketRequestLimit());

  return info;
}

} // namespace apache::thrift
