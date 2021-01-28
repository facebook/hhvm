<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function rx_context()[rx]: void {
  $more_permissive = ()[rx, policied] ==> {
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

function policied_context()[policied]: void {
  // the type-checker shouldn't close over the Output capability
  ()[rx] ==> policied_context(); // error
}

function nesting_test()[]: void {
  $rx_lambda = ()[rx] ==> {};
  $least_permissive = ()[] ==> {};
  $policied = ()[policied] ==> {};

  ()[policied] ==> {
    $call_lp = ()[] ==> $least_permissive();
    $call_rx = ()[rx] ==> {
      $rx_lambda(); // ok
      $least_permissive(); // ok
      $policied(); // error
    };
    ()[] ==> {
      $rx_lambda(); // error
      $call_rx(); // error (verify inner lambda is typed properly)

      $least_permissive(); // ok
      $call_lp(); // ok (verify inner lambda is typed properly)

      $policied(); // error
    };
  };
}
