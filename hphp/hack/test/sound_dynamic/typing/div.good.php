<?hh

<<__SupportDynamicType>>
function div_int_int(int $x, int $y) : num {
  return $x / $y;
}

<<__SupportDynamicType>>
function div_float_float(float $x, float $y) : float {
  return $x / $y;
}

<<__SupportDynamicType>>
function div_num_num(num $x, num $y) : num {
  return $x / $y;
}

<<__SupportDynamicType>>
function div_nothing_nothing(nothing $x, nothing $y) : nothing {
  return $x / $y;
}

<<__SupportDynamicType>>
function div_num_nothing(num $x, nothing $y) : nothing {
  return $x / $y;
}

<<__SupportDynamicType>>
function div_num_int(num $x, int $y) : num {
  return $x / $y;
}

<<__SupportDynamicType>>
function div_float_int(float $x, int $y) : float {
  return $x / $y;
}

<<__SupportDynamicType>>
function div_float_num(float $x, num $y) : float {
  return $x / $y;
}

function test(dynamic $d, int $i, float $f, num $n, vec<~int> $vi, vec<~float> $vf, vec<~num> $vn) : void {
  hh_expect_equivalent<num>($i / $i);
  hh_expect_equivalent<float>($f / $f);
  hh_expect_equivalent<num>($n / $n);
  hh_expect_equivalent<num>($n / $i);
  hh_expect_equivalent<float>($f / $i);
  hh_expect_equivalent<float>($f / $n);

  hh_expect_equivalent<~num>($vi[0] / $i);
  hh_expect_equivalent<~float>($vf[0] / $f);
  hh_expect_equivalent<~num>($vn[0] / $n);
  hh_expect_equivalent<~num>($vn[0] / $i);
  hh_expect_equivalent<~float>($vf[0] / $i);
  hh_expect_equivalent<~float>($vf[0] / $n);
  hh_expect_equivalent<~num>($vn[0] / $d);
  hh_expect_equivalent<~num>($vi[0] / $d);
  hh_expect_equivalent<~float>($vf[0] / $d);
  hh_expect_equivalent<~num>($d / $vn[0]);
  hh_expect_equivalent<~num>($d / $vi[0]);
  hh_expect_equivalent<~float>($d / $vf[0]);
  hh_expect_equivalent<dynamic>($d / $d);


  // All the expressions above should act like function calls to the correctly
  // overloaded function. It's ambiguous what to do in the dynamic case,
  // since dynamic = ~nothing, and there isn't an overloded case for nothing.
  // One could argue that the return should be nothing, and hence the result
  // should just be dynamic.
  hh_expect_equivalent<num>(div_int_int($i, $i));
  hh_expect_equivalent<float>(div_float_float($f, $f));
  hh_expect_equivalent<num>(div_num_num($n, $n));
  hh_expect_equivalent<num>(div_num_int($n, $i));
  hh_expect_equivalent<float>(div_float_int($f, $i));
  hh_expect_equivalent<float>(div_float_num($f, $n));

  hh_expect_equivalent<~num>(div_int_int($vi[0], $i));
  hh_expect_equivalent<~float>(div_float_float($vf[0], $f));
  hh_expect_equivalent<~num>(div_num_num($vn[0], $n));
  hh_expect_equivalent<~num>(div_num_int($vn[0], $i));
  hh_expect_equivalent<~float>(div_float_int($vf[0], $i));
  hh_expect_equivalent<~float>(div_float_num($vf[0], $n));
  hh_expect_equivalent<dynamic>(div_num_nothing($vn[0], $d));
  hh_expect_equivalent<dynamic>(div_nothing_nothing($d, $d));
}
