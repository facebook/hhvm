<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda')>>

function thisParseErrorIsFun(int $y): void {
  $_ = function<T>(T $x): T use [$x] { return tuple($x, $y);};
}
