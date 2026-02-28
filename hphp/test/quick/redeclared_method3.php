<?hh
interface I {
  function foo():mixed;
}
class C implements I {
  static function foo() :mixed{ echo "foo\n"; }
}
<<__EntryPoint>> function main(): void {
$obj = new C;
$obj->foo();
}
