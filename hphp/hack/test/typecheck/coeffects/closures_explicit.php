<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function rx_context()[rx]: void {
  $more_permissive = ()[rx, output] ==> {
    rx_context(); // ok
  };

  $less_permissive = ()[] ==> {
    $more_permissive(); // error (missing output)
    rx_context(); // error (pure </: rx)
  };

  $equally_permissive = ()[rx] ==> {
    $less_permissive(); // ok (rx <: pure)
    rx_context(); // ok
    $more_permissive(); // error (missing output)
  };
}

function output_context()[output]: void {
  // the type-checker shouldn't close over the Output capability
  ()[rx] ==> output_context(); // error
}

function nesting_test()[]: void {
  $rx_lambda = ()[rx] ==> {};
  $least_permissive = ()[] ==> {};
  $output = ()[output] ==> {};

  ()[output] ==> {
    $call_lp = ()[] ==> $least_permissive();
    $call_rx = ()[rx] ==> {
      $rx_lambda(); // ok
      $least_permissive(); // ok
      $output(); // error
    };
    ()[] ==> {
      $rx_lambda(); // error
      $call_rx(); // error (verify inner lambda is typed properly)

      $least_permissive(); // ok
      $call_lp(); // ok (verify inner lambda is typed properly)

      $output(); // error
    };
  };
}
