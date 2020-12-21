<?hh // partial
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// OK
<<__Rx>>
function f(int $a)[]: void {
  unset($a);
}
