<?hh

/**
 * Accessing fields/passing shapes to functions
 * doesn't change the "known fields" property
 */
type s = shape('z' => ?int);

function test(): s {
  $s = shape();
  $s['x'] = 'aaa';
  return $s;
}

type t = shape('x' => ?int);

function f(t $_): void {}

function test2(bool $b): s {
  $s = shape();
  if ($b) {
    $s['x'] = 0;
  } else {
    $s['y'] = '';
  }

  f($s);
  return $s;
}
