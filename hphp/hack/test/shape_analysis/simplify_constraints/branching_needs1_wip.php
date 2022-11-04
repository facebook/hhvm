<?hh

// expect shape() with no keys
function foo(dict<string, mixed> $d)
                  // TODO(T136668856): expect shape(a => mixed, b => mixed)
                  : dict<string, mixed> {
  if (true === false) {
    $d['a'];
    inspect($d);
  } else {
    $d['b'];
    inspect($d);
  }
  return $d;
}
