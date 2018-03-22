<?hh // strict

<<__Rx>>
function f(darray<int, Map<int, int>> $a): void {
  $a[0][5] = 5;
}
