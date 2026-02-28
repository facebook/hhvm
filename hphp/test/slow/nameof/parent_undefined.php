<?hh

class C {
  public static function test(): void {
    var_dump(nameof parent);
  }
}

<<__EntryPoint>>
function main(): void {
  C::test();
}
