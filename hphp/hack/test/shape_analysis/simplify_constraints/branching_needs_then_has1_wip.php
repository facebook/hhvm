<?hh

// solution is correct: empty shape() with no keys
function foo(dict<string, mixed> $d)
                  : dict<string, mixed> {
  if (true === false) {
    $d['a'];
    $d['a'] = 1;
    inspect($d);
  } else {
    $d['b'];
    $d['b'] = 1;
    inspect($d);
  }
  return $d;
}
