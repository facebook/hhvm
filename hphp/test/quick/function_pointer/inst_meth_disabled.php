<?hh

class Bar {
  public function baz(): void {}

  public static function baz2(): void {}
}

<<__EntryPoint>>
function test(): void {
  inst_meth(new Bar(), 'baz');
}
