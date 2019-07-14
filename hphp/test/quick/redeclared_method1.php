<?hh
interface I {
  public function foo();
}
class C implements I {
  protected function foo() { echo "foo\n"; }
  public function bar() { $this->foo(); }
}
<<__EntryPoint>> function main(): void {
$obj = new C;
$obj->bar();
}
