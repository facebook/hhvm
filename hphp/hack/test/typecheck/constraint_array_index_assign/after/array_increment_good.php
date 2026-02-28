<?hh

function my_example(): vec<mixed> {
  $x = vec[0];
  $x[0] += 1;
  return $x;
}
