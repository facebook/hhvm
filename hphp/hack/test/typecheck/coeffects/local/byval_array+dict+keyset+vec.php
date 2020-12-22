<?hh

function ok(vec<int> $a, vec<vec<int>> $b)[]: void {
  $a[0] = 1;
  $a[] = 2;
  $b[1][3] = 5;
  $b[0][] = 10;
}

function ok1(keyset<int> $a)[]: void {
  $a[] = 2;
}

function ok2(dict<int, int> $a, dict<int, varray<int>> $b)[]: void {
  $a[2] = 3;
  $b[2][3] = 5;
}

function ok3(varray<int> $a, varray<darray<int, int>> $b)[]: void {
  $a[2] = 3;
  $a[] = 2;
  $b[1][5] = 10;
}

function ok4(darray<int, int> $a)[]: void {
  $a[2] = 3;
}
