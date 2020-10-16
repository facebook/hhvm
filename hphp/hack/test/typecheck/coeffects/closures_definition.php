<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function at_least_rx((function()[rx]: void) $f)[pure]: void {}

function ok_rx()[rx]: (function()[rx]: void) {
  $l = () ==> {};
  at_least_rx($l);
  return $l;
}

function ok_pure()[pure]: (function()[pure]: void) {
  $l = () ==> {};
  at_least_rx($l); // ok: ()[pure] <: ()[rx]
  return $l;
}

// a context that unsafely provides capability RxLocal
function unsafe_context()[rx_shallow]: void {
  // verify that the lambda inherits RxLocal
  // in addition to the safe part: RxShallow
  () ==> needs_rx_local();
}

function needs_rx_local()[rx_local]: void {}

function at_least_rx_shallow((function ()[rx_shallow]: void) $f)[pure]: void {}

function f()[rx_shallow]: void {
  $g = () ==> needs_rx_local();
  at_least_rx_shallow($g);
}
