<?hh
interface I {
  static function foo();
}
class C implements I {
  function foo() { echo "foo\n"; }
}
<<__EntryPoint>> function main(): void {
$obj = new C;
$obj->foo();
}
