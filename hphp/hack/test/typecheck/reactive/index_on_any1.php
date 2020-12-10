<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>


<<__Rx>>
function g(varray<varray<int>> $a): void {
  $c = $a[0];
  $c[0] = 5;
}
