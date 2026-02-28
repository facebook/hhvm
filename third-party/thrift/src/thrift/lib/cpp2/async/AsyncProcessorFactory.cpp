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

#include <thrift/lib/cpp2/async/AsyncProcessorFactory.h>

#include <fmt/core.h>
#include <fmt/format.h>

namespace apache::thrift {

namespace {
const std::string kNONE = "NONE";

std::string priorityToString(concurrency::PRIORITY priority) {
  switch (priority) {
    case concurrency::PRIORITY::HIGH_IMPORTANT:
      return "HIGH_IMPORTANT";
    case concurrency::PRIORITY::HIGH:
      return "HIGH";
    case concurrency::PRIORITY::IMPORTANT:
      return "IMPORTANT";
    case concurrency::PRIORITY::NORMAL:
      return "NORMAL";
    case concurrency::PRIORITY::BEST_EFFORT:
      return "BEST_EFFORT";
    case concurrency::PRIORITY::N_PRIORITIES:
      return "N_PRIORITIES";
    default:
      folly::assume_unreachable();
  }
}

std::string executorTypeToString(
    AsyncProcessorFactory::MethodMetadata::ExecutorType executorType) {
  switch (executorType) {
    case AsyncProcessorFactory::MethodMetadata::ExecutorType::UNKNOWN:
      return "UNKNOWN";
    case AsyncProcessorFactory::MethodMetadata::ExecutorType::EVB:
      return "EVB";
    case AsyncProcessorFactory::MethodMetadata::ExecutorType::ANY:
      return "ANY";
    default:
      folly::assume_unreachable();
  }
}

std::string interactionTypeToString(
    AsyncProcessorFactory::MethodMetadata::InteractionType interactionType) {
  switch (interactionType) {
    case AsyncProcessorFactory::MethodMetadata::InteractionType::UNKNOWN:
      return "UNKNOWN";
    case AsyncProcessorFactory::MethodMetadata::InteractionType::NONE:
      return "NONE";
    case AsyncProcessorFactory::MethodMetadata::InteractionType::INTERACTION_V1:
      return "INTERACTION_V1";
    default:
      folly::assume_unreachable();
  }
}
} // namespace

std::string AsyncProcessorFactory::MethodMetadata::describeFields() const {
  return fmt::format(
      "executorType={} interactionType={} rpcKind={} priority={} "
      "interactionName={} createsInteraction={} isWildcard={}",
      executorTypeToString(executorType),
      interactionTypeToString(interactionType),
      rpcKind ? util::enumNameSafe(*rpcKind) : kNONE,
      priority ? priorityToString(*priority) : kNONE,
      interactionName.value_or(kNONE),
      createsInteraction,
      isWildcard());
}

std::string AsyncProcessorFactory::WildcardMethodMetadata::describe() const {
  return fmt::format("WildcardMethodMetadata({})", describeFields());
}

std::string AsyncProcessorFactory::MethodMetadata::describe() const {
  return fmt::format("MethodMetadata({})", describeFields());
}

std::string AsyncProcessorFactory::describe(
    const MethodMetadataMap& metadataMap) {
  auto buf = fmt::memory_buffer();
  auto inserter = std::back_inserter(buf);
  fmt::format_to(inserter, "MethodMetadataMap(");

  for (auto entry = cbegin(metadataMap); entry != cend(metadataMap); ++entry) {
    if (entry != cbegin(metadataMap)) {
      fmt::format_to(inserter, " ");
    }
    fmt::format_to(inserter, "{}={}", entry->first, entry->second->describe());
  }

  fmt::format_to(inserter, ")");
  return fmt::to_string(buf);
}

std::string AsyncProcessorFactory::describe(
    const WildcardMethodMetadataMap& metadataMap) {
  return fmt::format(
      "WildcardMethodMetadataMap(wildcardMetadata={} knownMethods={})",
      metadataMap.wildcardMetadata ? metadataMap.wildcardMetadata->describe()
                                   : kNONE,
      describe(metadataMap.knownMethods));
}

std::string AsyncProcessorFactory::describe(
    const CreateMethodMetadataResult& createMethodMetadataResult) {
  auto buf = fmt::memory_buffer();
  auto inserter = std::back_inserter(buf);
  fmt::format_to(inserter, "CreateMethodMetadataResult(");
  std::visit(
      [&inserter](auto&& arg) {
        fmt::format_to(inserter, "{}", describe(arg));
      },
      createMethodMetadataResult);
  fmt::format_to(inserter, ")");
  return fmt::to_string(buf);
}

} // namespace apache::thrift
