<?hh

// untyped
function f($a) {
  echo $a;
}

// partially typed array
function g(array $a) {
  echo $a;
}

// partially typed return value
async function h() {
  return 1;
}

// partially typed return value used here
function i() {
  echo h();
}

function strict(int $x): int {
  return $x;
}

function use_strict() {
  echo strict(1);
}

function has_unsafe(int $a) {
  echo $a;
  // UNSAFE
  echo $a;
}

function has_unsafeexpr(string $a) {
  echo /* UNSAFE_EXPR */ $a + 1;
}
