/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fb303/ThreadCachedServiceData.h>

namespace proxygen {

/*
 * Counter definitions for use in child classes.  Updating all
 * children thus becomes as simple as updating these definitions.
 * It is thus intended and recommended for all callers to refer to
 * BaseStats::<counter> when wishing to use counters.
 */
class BaseStats {
 private:
  // Private constructor so its clear nothing else should implement this class
  BaseStats() = default;

 public:
  // TODO: given the simple nature of TLCounter and that it is explicitly
  // thread safe via the use of atomics, we may only want single local
  // instance instead of wrapped (per thread) instances.
  using TLCounter = facebook::fb303::CounterWrapper;
  // Please avoid adding DynamicTimeseriesWrapper if we can.
  // At a minimum they require formatters and map lookups for
  // operations and make it easier to violate the constraint that all counters
  // are created at startup.
  using TLTimeseriesMinute = facebook::fb303::MinuteOnlyTimeseriesWrapper;
  // TLTimeseries was exporting as TimeseriesPolymorphicWrapper
  // were are trying to get rid of .600/.3600 counters
  // therefore aliasing it to TLTTimeSeriesMinute which only
  // exports .60 counters
  using TLTimeseries = TLTimeseriesMinute;
  using TLTimeseriesMinuteAndAllTime = facebook::fb303::MinuteTimeseriesWrapper;

  using TLHistogram = facebook::fb303::MinuteOnlyHistogram;
  using TLHistogramMinuteAndAllTime = facebook::fb303::HistogramWrapper;
  // Please avoid adding DynamicHistogramWrapper if we can.
  // At a minimum they require formatters and map lookups for
  // operations and make it easier to violate the constraint that all counters
  // are created at startup.
};

} // namespace proxygen
