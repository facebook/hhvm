<?hh // partial

function varray<T>(mixed $x): varray<T> {
  $result = varray[];
  foreach ((array)$x as $v) {
    $result[] = $v;
  }
  return $result;
}

function testVarray($x): varray<int> {
  return varray($x);
}
