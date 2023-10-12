<?hh //strict

function read_from_append(varray<int> $x): void {
  $x[] = 4;

  $y = $x[];
}
