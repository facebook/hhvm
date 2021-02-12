<?hh // strict

function f(vec<int> $a, vec<vec<int>> $b)[]: void {
  $a[0] = 1;
  $a[] = 2;
  $b[1][3] = 5;
  $b[0][] = 10;
}


function f1(keyset<int> $a)[]: void {
  $a[] = 2;
}


function f2(dict<int, int> $a, dict<int, varray<int>> $b)[]: void {
  $a[2] = 3;
  $b[2][3] = 5;
}


function f3(varray<int> $a, varray<darray<int, int>> $b)[]: void {
  $a[2] = 3;
  $a[] = 2;
  $b[1][5] = 10;
}


function f4(darray<int, int> $a)[]: void {
  $a[2] = 3;
}
