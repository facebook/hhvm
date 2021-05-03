<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function at_least_rx((function()[oldrx]: void) $f)[]: void {}

function ok_rx()[oldrx]: (function()[oldrx]: void) {
  $l = () ==> {};
  at_least_rx($l);
  return $l;
}

function ok_pure()[]: (function()[]: void) {
  $l = () ==> {};
  at_least_rx($l); // ok: ()[] <: ()[oldrx]
  return $l;
}

// a context that unsafely provides capability RxLocal
function unsafe_context()[oldrx_shallow]: void {
  // verify that the lambda inherits RxLocal
  // in addition to the safe part: RxShallow
  () ==> needs_rx_local();
}

function needs_rx_local()[oldrx_local]: void {}

function at_least_rx_shallow((function ()[oldrx_shallow]: void) $f)[]: void {}

function f()[oldrx_shallow]: void {
  $g = () ==> needs_rx_local();
  at_least_rx_shallow($g);
}
