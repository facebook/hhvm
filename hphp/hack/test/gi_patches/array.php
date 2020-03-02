<?hh // partial

function f(array $v) {
  $v[3] = "foo";
  return $v[34897];
}

function g(array $v) {
  expect<string>($v[0]);
}

function h1(): array {
  return array(0 => 'billie');
}

function h2(): array {
  return array('billie');
}

function i(): array {
  $x = array();
  $x[3] = 'bob';
  return $x;
}

function expect<T>(T $_): void {}
