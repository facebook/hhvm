<?hh

function f(nonnull $x): bool {
  return $x === 42;
}

function g(nonnull $x): bool {
  return $x !== 'foo';
}
