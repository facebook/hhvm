<?hh
class B {
  public function foo() :mixed{
    echo isset($this);
    echo "#\n";
  }
}

<<__EntryPoint>> function main(): void {
  $b = new B;
  $b->foo();
}
