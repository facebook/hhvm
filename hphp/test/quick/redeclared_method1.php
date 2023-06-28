<?hh
interface I {
  public function foo():mixed;
}
class C implements I {
  protected function foo() :mixed{ echo "foo\n"; }
  public function bar() :mixed{ $this->foo(); }
}
<<__EntryPoint>> function main(): void {
$obj = new C;
$obj->bar();
}
