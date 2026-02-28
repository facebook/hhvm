<?hh

<<file: __EnableUnstableFeatures('case_types')>>

case type CT = int | (function(): int);

function f(CT $c): int {
  if ($c is Closure) {
    return $c();
  } else if ($c is int) {
    return $c;
  } else {
    return $c();
  }
}
