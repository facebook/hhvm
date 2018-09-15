<?hh // strict

<<__Rx>>
function f<Tv as arraykey>(bool $c, Tv $v1, Tv $v2): keyset<Tv> {
  $result = keyset[];
  if ($c) {
    $result[] = $v1;
  } else {
    $result[] = $v2;
  }
  $result[] = $v1;
  return $result;
}
