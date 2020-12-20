<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f(varray<Map<int, int>> $a)[rx]: void {
  $a[0][1] = 5;
}
