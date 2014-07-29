<?hh

class C {
  public async function f() {
    return 10;
  }
}

function test(): int {
  $inst = new C();
  $x = 10;
  if (!$inst->f()) {
    $x = 20;
  }
  return $x;
}
