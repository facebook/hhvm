<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function rx_context()[oldrx]: void {
  $more_permissive = ()[oldrx, unrelated] ==> {
    rx_context(); // ok
  };

  $less_permissive = ()[] ==> {
    $more_permissive(); // error (missing unrelated)
    rx_context(); // error (pure </: oldrx)
  };

  $equally_permissive = ()[oldrx] ==> {
    $less_permissive(); // ok (oldrx <: pure)
    rx_context(); // ok
    $more_permissive(); // error (missing unrelated)
  };
}

function unrelated_context()[unrelated]: void {
  // the type-checker shouldn't close over the Unrelated capability
  ()[oldrx] ==> unrelated_context(); // error
}

function nesting_test()[]: void {
  $rx_lambda = ()[oldrx] ==> {};
  $least_permissive = ()[] ==> {};
  $unrelated = ()[unrelated] ==> {};

  ()[unrelated] ==> {
    $call_lp = ()[] ==> $least_permissive();
    $call_oldrx = ()[oldrx] ==> {
      $rx_lambda(); // ok
      $least_permissive(); // ok
      $unrelated(); // error
    };
    ()[] ==> {
      $rx_lambda(); // error
      $call_oldrx(); // error (verify inner lambda is typed properly)

      $least_permissive(); // ok
      $call_lp(); // ok (verify inner lambda is typed properly)

      $unrelated(); // error
    };
  };
}
