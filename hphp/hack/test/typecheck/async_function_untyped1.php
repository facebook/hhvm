<?hh

async function f() {
  return 10;
}

function test(): int {
  $x = 10;
  if (f()) {
    $x = 20;
  }
  return $x;
}
