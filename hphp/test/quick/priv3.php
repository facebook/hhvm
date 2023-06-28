<?hh
class C {
  private function foo() :mixed{
    echo "C::foo\n";
  }
  public function test() :mixed{
    // This should not call C::foo, it should fatal
    D::foo();
  }
}
class D extends C {
  private function foo() :mixed{
    echo "D::foo\n";
  }
}
<<__EntryPoint>> function main(): void {
$obj = new D;
$obj->test();
}
