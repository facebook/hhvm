<?hh

class C {
  public static function f() {
    return "lol";
  }
}

function test(): void {
  $_ = ($x = vec[C::f()]): void ==> {};
}
