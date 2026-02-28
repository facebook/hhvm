<?hh

class C {
  public static function foo(): void {}
}

function test(): void {
  $cls = C::class;
  $cls::foo();
}
