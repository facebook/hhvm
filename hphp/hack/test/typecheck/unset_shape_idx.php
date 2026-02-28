<?hh

type s = shape('blah' => int);
function f(s $s): int {
  // even though we allow unset on *array* indexes,
  // we shouldn't allow it on shapes
  unset($s['blah']);
  return $s['blah'];
}
