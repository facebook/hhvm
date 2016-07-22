<?hh

function foo(): keyset<int> {
  $x = keyset[1, 2, 3];
  return $x;
}
