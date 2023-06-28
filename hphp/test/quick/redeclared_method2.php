<?hh
interface I {
  static function foo():mixed;
}
class C implements I {
  function foo() :mixed{ echo "foo\n"; }
}
<<__EntryPoint>> function main(): void {
$obj = new C;
$obj->foo();
}
