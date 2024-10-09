<?hh
<<file:__EnableUnstableFeatures('like_type_hints')>>

function f(
  ~int $a,
  vec<~dict<int, string>> $b,
  ~SomeClass::TFoo $c,
  (function(~int): void) $d,
  ?~int $e,
  ~?int $f,
  ~~int $g,
  (int, ~string) $h,
): void {}
