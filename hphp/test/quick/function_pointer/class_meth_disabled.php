<?hh

class C {
  public static function bar(): void {}
}

<<__EntryPoint>>
function test(): void {
  class_meth('C', 'bar');
}
