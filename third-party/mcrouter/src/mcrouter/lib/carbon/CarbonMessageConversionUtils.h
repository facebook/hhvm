/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/json/dynamic.h>

namespace carbon {

struct FollyDynamicConversionOptions {
  /**
   * If false, each mixin will be put in the object as a member with name
   * "__MixinAliasOrName".
   *
   * If true, members of mixins would be put directly into the object as if
   * they were direct members.
   */
  bool inlineMixins{false};

  /**
   * If false, all fields that are not serializable would contain
   * "(not serializable)" as a value.
   *
   * If true, such fields would be omitted from the output completely.
   */
  bool ignoreUnserializableTypes{false};

  /**
   * If true, all fields will be serialized (including empty strings and
   * integers with value == 0).
   *
   * If false, such fields will be omitted from the output completely.
   */
  bool serializeFieldsWithDefaultValue{true};
};

/**
 * Convenience method for converting Carbon generated messages/structures into
 * folly::dynamic.
 *
 * Note: this method is limited in what it can convert, all unknown types will
 *       be not properly converted and will be replaced with
 *       "(not serializable)".
 */
template <class Message>
folly::dynamic convertToFollyDynamic(
    const Message& m,
    FollyDynamicConversionOptions opts = FollyDynamicConversionOptions());

/**
 * Convenience method for filling a carbon struct with a folly::dynamic
 * Note: Works fine with both inlineMixins configurations.
 *
 * @param json    The folly::dynamic that will be used to fill the message.
 * @param m       Output argument with the instance of the Message to be filled.
 * @param onError Callback that is called whenever an error happens.
 *                If no callback is given, all errors are ignored.
 */
template <class Message>
void convertFromFollyDynamic(
    const folly::dynamic& json,
    Message& m,
    std::function<void(folly::StringPiece fieldName, folly::StringPiece msg)>
        onError = nullptr);

} // namespace carbon

#include "CarbonMessageConversionUtils-inl.h"
