<?hh
<<__SupportDynamicType>>
function minus_int(int $x) : int {
  return -$x;
}

<<__SupportDynamicType>>
function minus_float(float $x) : float {
  return -$x;
}

<<__SupportDynamicType>>
function minus_num(num $x) : num {
  return -$x;
}

<<__SupportDynamicType>>
function minus_nothing(nothing $x) : nothing {
  return -$x;
}

function test(dynamic $d, int $i, float $f, num $n, vec<~int> $vi, vec<~float> $vf, vec<~num> $vn) : void {
  $va = Vector{};
  hh_expect_equivalent<int>(-$i);
  hh_expect_equivalent<float>(-$f);
  hh_expect_equivalent<num>(-$n);
  hh_expect_equivalent<~int>(-$vi[0]);
  hh_expect_equivalent<~float>(-$vf[0]);
  hh_expect_equivalent<~num>(-$vn[0]);
  hh_expect_equivalent<dynamic>(-$d);
  // Expect an unresolved type to default to int for unary minus
  hh_expect<~int>(-$va[0]);

  // All the expressions above should act like function calls to the correctly
  // overloaded function. It's ambiguous what to do in the -dynamic case,
  // since dynamic = ~nothing, and there isn't an overloded case for nothing.
  // One could argue that the return should be nothing, and hence the result
  // should just be dynamic.
  hh_expect_equivalent<int>(minus_int($i));
  hh_expect_equivalent<float>(minus_float($f));
  hh_expect_equivalent<num>(minus_num($n));
  hh_expect_equivalent<~int>(minus_int($vi[0]));
  hh_expect_equivalent<~float>(minus_float($vf[0]));
  hh_expect_equivalent<~num>(minus_num($vn[0]));
  hh_expect_equivalent<dynamic>(minus_nothing($d));
}
