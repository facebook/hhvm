<?hh

class C {
  public static ?int $foo;
}

function test() {
  C::$foo;
}
