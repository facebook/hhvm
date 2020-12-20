<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f(<<__Mutable>>Vector<string> $m)[rx]: void {
  // ERROR
  $m[0] = "42";
}
