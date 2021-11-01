<?hh

class C {
  public static async function f() /* : TAny */ {
    return 10;
  }
}

function test(): int {
  $x = 10;
  if (C::f()) {
    $x = 20;
  }
  return $x;
}
