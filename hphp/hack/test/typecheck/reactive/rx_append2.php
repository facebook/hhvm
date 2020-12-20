<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function a(Set<int> $a)[rx]: void {
  $a[] = 1;
}
