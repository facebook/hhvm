<?hh

class C {
  public static function test(): void {
    var_dump(nameof self);
  }
}

<<__EntryPoint>>
function main(): void {
  C::test();
}
