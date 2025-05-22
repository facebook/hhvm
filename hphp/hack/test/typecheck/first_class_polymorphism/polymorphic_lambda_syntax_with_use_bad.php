<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda')>>

function thisParseErrorIsFun(int $y): void {
  $_ = function<T>(T $x): (T,int) use($y) ==> { return tuple($x, $y);};
}
