<?hh
interface I {
  function foo();
}
class C implements I {
  static function foo() { echo "foo\n"; }
}
<<__EntryPoint>> function main(): void {
$obj = new C;
$obj->foo();
}
