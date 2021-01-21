<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function default_context()[defaults]: void {}

function rx_context()[rx]: void {
  // verify the lambda inherits capability rx from the outer function
  $rx_lambda = () ==> rx_context(); // ok
  $rx_lambda2 = () ==> default_context(); // error
}

function implicit_context(): void {
  // the lambda should have defaults capability
  $default_lambda = () ==> {
    implicit_context(); // ok
    default_context(); // ok
    rx_context(); // ok (Rx <: Defaults)
  };

  $rx_lambda = ()[rx] ==> {};
  $default_lambda = () ==> $rx_lambda(); // ok
}

function cipp_context()[cipp_global]: void {
  // the lambda should be typed as having Output capability, not defaults
  () ==> rx_context(); // error

  ()[rx] ==> {
    // the type-checker shouldn't close over the output capability
    () ==> cipp_context(); // error FIXME(coeffects)

    () ==> rx_context(); // ok (since rx is in the enclosing scope / inherited)
  };
}
