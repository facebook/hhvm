<?hh

function always_true() :mixed{
  // Try to defeat any possible inlining / constant folding.
  return mt_rand(1, 2) < 10;
}

function a_nan() :mixed{
  return (int)NAN;
}

function b_nan() :mixed{
  $x = NAN;
  $y = (int)$x;
  return $y;
}

function get_nan() :mixed{
  if (always_true()) {
    return NAN;
  } else {
    return 0;
  }
}

function c_nan() :mixed{
  return (int)get_nan();
}

function a_inf() :mixed{
  return (int)INF;
}

function b_inf() :mixed{
  $x = INF;
  $y = (int)$x;
  return $y;
}

function get_inf() :mixed{
  if (always_true()) {
    return INF;
  } else {
    return 0;
  }
}

function c_inf() :mixed{
  return (int)get_inf();
}
<<__EntryPoint>>
function entrypoint_nan_inf_cast(): void {

  var_dump(a_nan());
  var_dump(b_nan());
  var_dump(c_nan());
  var_dump(a_inf());
  var_dump(b_inf());
  var_dump(c_inf());
}
