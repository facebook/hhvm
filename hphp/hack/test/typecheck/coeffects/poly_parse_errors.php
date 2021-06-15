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

abstract class V { abstract const ctx C; }

function var_improper_form(
  shape() $x,
  V ...$y
)[
  $x::C,
  $y::C
]: void {}

<<__Memoize>>
function memo(V $v)[$v::C]: void {}

class C {
  <<__Memoize>>
  public function memo(V $v)[$v::C]: void {}
}

function lambdas(): void {
  $x = ($f)[ctx $f] ==> {};
  $y = ($a)[$a::C] ==> {};
}
