<?hh

function my_example(): vec<mixed> {
  $x = vec[];
  $x[] += 1;
  return $x;
}
