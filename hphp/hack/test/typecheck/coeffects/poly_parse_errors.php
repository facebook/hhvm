<?hh

function missing_param()[
  ctx $f,
  $g::C
]: void {}

function missing_hint(
  $f,
  $g,
)[
  ctx $f,
  $g::C
]: void {}

function fun_improper_form(
  int $f,
  (function (): void) $g,
  (function ()[]: void) $h,
  (function ()[defaults]: void) $j,
)[
  ctx $f,
  ctx $g,
  ctx $h,
  ctx $j
]: void {}

function var_improper_form(
  shape() $x,
  vec<int> ...$y
)[
  $x::C,
  $y::C
]: void {}

<<__Memoize>>
function memo(vec<int> $v)[$v::C]: void {}

class C {
  <<__Memoize>>
  public function memo(vec<int> $v)[$v::C]: void {}
}

function lambdas(): void {
  $x = ($f)[ctx $f] ==> {};
  $y = ($a)[$a::C] ==> {};
}
