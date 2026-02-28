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

// @lint-ignore-every THRIFTFORMAT

include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace py thrift.compiler.test.fixtures.json_experimental.src.DemoWidgetDocs
namespace php thrift_php_demo

/**
  * What kinds of widgets can we buy and sell?
  */
enum WidgetType {
  /**  Any small device or object (usually hand-sized) which can be manipulated.
  */
  FROB = 1,
  /** The kind that you can use to open a door. */
  KNOB = 2,
  /** An actual person named Bob. */
  BOB = 3,
  JOB = 4,
  ROB = 5, ///< C++ style comments, also
///< and support for multiline
}

/**
 * We are looking to buy / sell some quantity of widgets,
 * subject to price and quantity constraints.
 *
 * @should min(abs(minWidgets), abs(maxWidgets)) * minPrice <= budget,
 *         'Your requisition can never be satisfied'
 */
struct WidgetRequisition {
  1: WidgetType type;
  /**
    How much can we spend on this order of widgets?
    @must _ > 0
   */
  2: i32 budgetCents;
  /** Negative quantities represent sale requisitions. */
  3: i32 minWidgets;
  /** @must minWidgets <= _ */
  4: i32 maxWidgets;
  /** A lower limit on the price makes sense if our logistics are not set
      up to handle massive quantities of cheap stuff.
      @must _ >= 0
   */
  5: i32 minPriceCents = 0;
  /** Our physical security can't deal with high-value items.
      @must minPrice <= _
   */
  6: i32 maxPriceCents;
}

/** Once a WidgetRequisition is fulfilled, it becomes an order.

    @must abs(numWidgets) * priceCents <= requisition.budget, 'Over budget'
    @must requisition.minWidgets <= numWidgets and
          numWidgets <= requisition.maxWidgets
    @must requisition.minPriceCents <= priceCents and
          priceCents <= requisition.maxPriceCents
 */
struct WidgetOrder {
  /** The requisition that generated this order. */
  1: WidgetRequisition requisition;
  /** Negative quantities represent sale orders. */
  2: i32 numWidgets;
  /** @must _ >= 0 */
  3: i32 priceCents;
  4: i32 numComments; ///< count of comments
  5: string comments;
}
