<?hh

function my_example(): vec<vec<int>> {
  $x = vec[vec[]];
  $x[0][] = 1;
  return $x;
}
