<?hh // strict

<<__Rx>>
function f<Tv as arraykey>(bool $c, Tv $v): mixed {
  $result = varray[];
  if ($c) {
    $result[] = $v;
  } else {
    $result = Vector { $v };
  }
  // ERROR
  $result[0] = $v;
  return $result;
}
