<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f(<<__Mutable>>Map<int, string> $m): void {
  // ERROR
  $m[1] = "42";
}
