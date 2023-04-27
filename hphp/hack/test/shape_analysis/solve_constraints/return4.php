<?hh

// Expect return hint with `a` key
function foo(): dict<string, mixed> {
  $d1 = dict['a' => 1];
  $d2 = dict_identity($d1);
  return $d2;
}


// Expect return hint with `b` key
function bar(): dict<string, mixed> {
  $d1 = dict['b' => 1];
  $d2 = dict_identity($d1);
  return $d2;
}

// expect hints that type-check
function dict_identity(dict<string, mixed> $d): dict<string, mixed> {
  return $d;
}
