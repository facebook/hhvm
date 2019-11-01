<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function setTimeStart(arraykey $ak):void { }

function maxva<T as num>(T $x, T $y): T {  return $x; }
function testit(int $cutoff_time, ?int $start):void {
  /* HH_FIXME[4323]  Exposed by adding return types to partial mode files */
  setTimeStart(maxva<_>($cutoff_time, $start));
}
