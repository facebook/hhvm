<?hh // partial
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// OK
<<__Rx>>
function f(int $a)[rx]: void {
  unset($a);
}
