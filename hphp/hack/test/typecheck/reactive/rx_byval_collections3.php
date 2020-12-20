<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f(darray<int, Map<int, int>> $a)[rx]: void {
  $a[0][5] = 5;
}
