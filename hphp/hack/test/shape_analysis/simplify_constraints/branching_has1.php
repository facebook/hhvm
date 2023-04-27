<?hh

// solution is correct: empty shape() with no keys
function foo(dict<string, mixed> $d)
                  // solution is correct: shape(?a => mixed, ?b => mixed)
                  : dict<string, mixed> {
  if (true === false) {
    $d['a'] = 1;
    inspect($d);
  } else {
    $d['b'] = 1;
    inspect($d);
  }
  return $d;
}
