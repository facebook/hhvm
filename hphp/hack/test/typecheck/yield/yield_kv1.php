<?hh

function f(): Generator<string, int, void> {
  yield 'one' => 1;
}

/* string is a subtype of arraykey */
function g(): Generator<arraykey, int, void> {
  yield 'two' => 2;
}

/* So is int, implied key */
function h(): Generator<arraykey, bool, void> {
  yield false;
  yield true;
}
