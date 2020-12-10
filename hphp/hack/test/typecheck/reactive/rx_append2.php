<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function a(Set<int> $a): void {
  $a[] = 1;
}
