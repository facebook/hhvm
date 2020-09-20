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
