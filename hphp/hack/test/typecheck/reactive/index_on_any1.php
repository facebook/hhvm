<?hh


function g(varray<varray<int>> $a): void {
  $c = $a[0];
  $c[0] = 5;
}
