<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function rx_context()[rx]: void {
  $more_permissive = ()[rx, non_det] ==> {
    rx_context(); // ok
  };

  $less_permissive = ()[pure] ==> {
    $more_permissive(); // error (missing non_det)
    rx_context(); // error (pure </: rx)
  };

  $equally_permissive = ()[rx] ==> {
    $less_permissive(); // ok (rx <: pure)
    rx_context(); // ok
    $more_permissive(); // error (missing non_det)
  };
}

function nondeterministic_context()[non_det]: void {
  // the type-checker shouldn't close over the non_det capability
  ()[rx] ==> nondeterministic_context(); // error
}

function nesting_test()[]: void {
  $rx_lambda = ()[rx] ==> {};
  $least_permissive = ()[] ==> {};
  $nondeterministic = ()[non_det] ==> {};

  ()[non_det] ==> {
    $call_lp = ()[] ==> $least_permissive();
    $call_rx = ()[rx] ==> {
      $rx_lambda(); // ok
      $least_permissive(); // ok
      $nondeterministic(); // error
    };
    ()[] ==> {
      $rx_lambda(); // error
      $call_rx(); // error (verify inner lambda is typed properly)

      $least_permissive(); // ok
      $call_lp(); // ok (verify inner lambda is typed properly)

      $nondeterministic(); // error
    };
  };
}
