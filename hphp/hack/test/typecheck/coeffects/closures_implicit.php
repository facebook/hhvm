<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function default_context()[defaults]: void {}

function rx_context()[oldrx]: void {
  // verify the lambda inherits capability Rx from the outer function
  $rx_lambda = () ==> rx_context(); // ok
  $rx_lambda2 = () ==> default_context(); // error
}

function implicit_context(): void {
  // the lambda should have defaults capability
  $default_lambda = () ==> {
    implicit_context(); // ok
    default_context(); // ok
    rx_context(); // ok (Rx <: defaults)
  };

  $rx_lambda = ()[oldrx] ==> {};
  $default_lambda = () ==> $rx_lambda(); // ok
}

function unrelated_context()[unrelated]: void {
  // the lambda should be typed as having Unrelated capability, not defaults
  () ==> rx_context(); // error

  ()[oldrx] ==> {
    // the type-checker shouldn't close over unrelated
    () ==> unrelated_context(); // error

    () ==> rx_context(); // ok (since Rx is in the enclosing scope / inherited)
  };
}
