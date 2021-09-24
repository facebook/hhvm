<?hh

function f(dynamic $d, int $i, vec<int> $v, Vector<int> $v2) : void {
  $d[$i];
  $d[$d];
  $v[$d];
  $v2[$d];

  $d[$i] = 1;
  $d[$d] = 1;
  $v[$d] = 1;
  $v2[$d] = 1;
  $v[$d] = $d;
}
