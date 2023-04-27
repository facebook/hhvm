<?hh

// expected parameter hint: shape()
// we infer incorrect parameter hint: shape('a' => mixed, 'b' => mixed)
function foo(dict<string, mixed> $d)
                  // inferred return hint is correct: shape(?a => mixed, ?b => mixed)
                  : dict<string, mixed> {
  if (true === false) {
    $d['a'] = 1;
    $d['a'];
    inspect($d);
  } else {
    $d['b'] = 1;
    $d['b'];
    inspect($d);
  }
  return $d;
}
