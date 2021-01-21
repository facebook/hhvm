<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function rx_context()[rx]: void {
  $more_permissive = ()[rx, cipp_global] ==> {
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

function cipp_global_context()[cipp_global]: void {
  // the type-checker shouldn't close over the Output capability
  ()[rx] ==> cipp_global_context(); // error
}

function nesting_test()[]: void {
  $rx_lambda = ()[rx] ==> {};
  $least_permissive = ()[] ==> {};
  $cipp_global = ()[cipp_global] ==> {};

  ()[cipp_global] ==> {
    $call_lp = ()[] ==> $least_permissive();
    $call_rx = ()[rx] ==> {
      $rx_lambda(); // ok
      $least_permissive(); // ok
      $cipp_global(); // error
    };
    ()[] ==> {
      $rx_lambda(); // error
      $call_rx(); // error (verify inner lambda is typed properly)

      $least_permissive(); // ok
      $call_lp(); // ok (verify inner lambda is typed properly)

      $cipp_global(); // error
    };
  };
}
