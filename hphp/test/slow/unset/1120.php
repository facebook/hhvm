<?hh

class A {
  public function foo() :mixed{
    unset($this);
    var_dump($this);
  }
}
<<__EntryPoint>> function main(): void {
$obj = new A;
$obj->foo();
}
