<?hh //strict

function read_from_append(array<int> $x): void {
  $x[] = 4;

  $y = $x[];
}
