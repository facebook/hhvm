<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function default_context()[non_rx]: void {}

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

function nondeterministic_context()[non_det]: void {
  // the lambda should be typed as having non_det capability, not defaults
  () ==> rx_context(); // error

  ()[rx] ==> {
    // the type-checker shouldn't close over the non_det capability
    () ==> nondeterministic_context(); // error

    () ==> rx_context(); // ok (since rx is in the enclosing scope / inherited)
  };
}
