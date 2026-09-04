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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ExtensionSlots.h>

#include <string>

#include <gtest/gtest.h>

using namespace apache::thrift::fast_thrift::thrift;

namespace {

struct AlphaExtension {
  EXTENSION_ID(alpha);
  struct ConnState {
    int value{7};
  };
  struct RequestState {
    int value{11};
  };
};

struct BetaExtension {
  EXTENSION_ID(beta);
  struct ConnState {
    std::string value{"beta"};
  };
};

struct GammaExtension {
  EXTENSION_ID(gamma);
  struct ConnState {
    int value{0};
  };
};

struct DeltaExtension {
  EXTENSION_ID(delta);
  struct ConnState {
    int value{0};
  };
};

ExtensionLayout layoutOf(std::initializer_list<ExtensionId> ids) {
  ExtensionLayoutBuilder builder;
  for (auto id : ids) {
    builder.add(id);
  }
  return std::move(builder).build();
}

} // namespace

TEST(ExtensionSlotsTest, IdsAreDistinctAndCompileTimeConstants) {
  EXPECT_NE(AlphaExtension::kId, BetaExtension::kId);
  static_assert(AlphaExtension::kId != BetaExtension::kId);
}

TEST(ExtensionSlotsTest, UninstalledSlotsFindNothing) {
  ExtensionSlots slots;
  EXPECT_EQ(
      slots.find<AlphaExtension::ConnState>(AlphaExtension::kId), nullptr);
  EXPECT_FALSE(slots.installed(AlphaExtension::kId));
}

TEST(ExtensionSlotsTest, EmptyLayoutInstallsNothing) {
  const ExtensionLayout layout = layoutOf({});
  EXPECT_TRUE(layout.empty());
  EXPECT_EQ(layout.slotCount(), 0);

  ExtensionSlots slots;
  slots.install(layout);
  EXPECT_EQ(
      slots.find<AlphaExtension::ConnState>(AlphaExtension::kId), nullptr);
}

// An installed slot starts empty: the framework reserves it, the extension
// decides when there is anything to publish.
TEST(ExtensionSlotsTest, InstalledSlotStartsNull) {
  const ExtensionLayout layout = layoutOf({AlphaExtension::kId});
  ExtensionSlots slots;
  slots.install(layout);

  EXPECT_TRUE(slots.installed(AlphaExtension::kId));
  EXPECT_EQ(
      slots.find<AlphaExtension::ConnState>(AlphaExtension::kId), nullptr);
}

TEST(ExtensionSlotsTest, PublishedStateIsFoundAndNotCopied) {
  const ExtensionLayout layout = layoutOf({AlphaExtension::kId});
  ExtensionSlots slots;
  slots.install(layout);

  AlphaExtension::ConnState owned;
  slots.set(AlphaExtension::kId, &owned);

  auto* found = slots.find<AlphaExtension::ConnState>(AlphaExtension::kId);
  ASSERT_EQ(found, &owned);
  owned.value = 42;
  EXPECT_EQ(found->value, 42);
}

TEST(ExtensionSlotsTest, ClearingASlotLeavesNothingToRead) {
  const ExtensionLayout layout = layoutOf({AlphaExtension::kId});
  ExtensionSlots slots;
  slots.install(layout);

  AlphaExtension::ConnState owned;
  slots.set(AlphaExtension::kId, &owned);
  ASSERT_NE(
      slots.find<AlphaExtension::ConnState>(AlphaExtension::kId), nullptr);

  slots.set(AlphaExtension::kId, nullptr);
  EXPECT_EQ(
      slots.find<AlphaExtension::ConnState>(AlphaExtension::kId), nullptr);
}

TEST(ExtensionSlotsTest, EachExtensionGetsItsOwnSlot) {
  const ExtensionLayout layout =
      layoutOf({AlphaExtension::kId, BetaExtension::kId});
  ExtensionSlots slots;
  slots.install(layout);

  AlphaExtension::ConnState alpha;
  BetaExtension::ConnState beta;
  slots.set(AlphaExtension::kId, &alpha);
  slots.set(BetaExtension::kId, &beta);

  EXPECT_EQ(slots.find<AlphaExtension::ConnState>(AlphaExtension::kId), &alpha);
  EXPECT_EQ(slots.find<BetaExtension::ConnState>(BetaExtension::kId), &beta);
}

TEST(ExtensionSlotsTest, InstallOrderDoesNotChangeWhatALookupFinds) {
  const ExtensionLayout forward =
      layoutOf({AlphaExtension::kId, BetaExtension::kId});
  const ExtensionLayout reverse =
      layoutOf({BetaExtension::kId, AlphaExtension::kId});

  AlphaExtension::ConnState alpha;
  ExtensionSlots forwardSlots;
  forwardSlots.install(forward);
  forwardSlots.set(AlphaExtension::kId, &alpha);

  ExtensionSlots reverseSlots;
  reverseSlots.install(reverse);
  reverseSlots.set(AlphaExtension::kId, &alpha);

  EXPECT_EQ(
      forwardSlots.find<AlphaExtension::ConnState>(AlphaExtension::kId),
      reverseSlots.find<AlphaExtension::ConnState>(AlphaExtension::kId));
}

TEST(ExtensionSlotsTest, AnExtensionAbsentFromTheLayoutFindsNothing) {
  const ExtensionLayout layout = layoutOf({AlphaExtension::kId});
  ExtensionSlots slots;
  slots.install(layout);
  EXPECT_FALSE(slots.installed(BetaExtension::kId));
  EXPECT_EQ(slots.find<BetaExtension::ConnState>(BetaExtension::kId), nullptr);
}

// Past the inline capacity the array spills to the heap; nothing about the
// contract changes, which is what makes the inline size a threshold rather
// than a limit.
TEST(ExtensionSlotsTest, MoreExtensionsThanInlineCapacityStillWork) {
  ASSERT_GT(4u, ExtensionSlots::kInlineSlots);
  const ExtensionLayout layout = layoutOf(
      {AlphaExtension::kId,
       BetaExtension::kId,
       GammaExtension::kId,
       DeltaExtension::kId});
  EXPECT_EQ(layout.slotCount(), 4);

  ExtensionSlots slots;
  slots.install(layout);

  AlphaExtension::ConnState alpha;
  DeltaExtension::ConnState delta;
  slots.set(AlphaExtension::kId, &alpha);
  slots.set(DeltaExtension::kId, &delta);

  EXPECT_EQ(slots.find<AlphaExtension::ConnState>(AlphaExtension::kId), &alpha);
  EXPECT_EQ(slots.find<DeltaExtension::ConnState>(DeltaExtension::kId), &delta);
  EXPECT_EQ(slots.find<BetaExtension::ConnState>(BetaExtension::kId), nullptr);
}

TEST(ExtensionSlotsDeathTest, DuplicateIdsAbortAtInstall) {
  ExtensionLayoutBuilder builder;
  builder.add(AlphaExtension::kId);
  EXPECT_DEATH(
      builder.add(AlphaExtension::kId),
      "two fast_thrift extensions share one id");
}

TEST(ExtensionSlotsDeathTest, PublishingToAnUninstalledExtensionAborts) {
  const ExtensionLayout layout = layoutOf({AlphaExtension::kId});
  ExtensionSlots slots;
  slots.install(layout);

  BetaExtension::ConnState beta;
  EXPECT_DEATH(
      slots.set(BetaExtension::kId, &beta), "extension is not installed");
}
