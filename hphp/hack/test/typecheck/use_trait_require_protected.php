<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait TDailyTimeRange {
  require extends TimeRangeBase;
}
final class TimeRange extends TimeRangeBase {
  use TDailyTimeRange;
}
abstract class TimeRangeBase {
  protected function foo():void { }
}
final class DailyTimeRange extends TimeRangeBase {

  public function fromTimeRange(
    TimeRange $time_range,
  ): void {
    $time_range->foo();
  }
}
