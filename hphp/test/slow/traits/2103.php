<?hh

trait TraitFoo {
  public function testDoSomethingInTrait() {
    return $this->doSomethingInTrait();
  }
  public function testDoSomethingPublicInTrait() {
    return $this->doSomethingPublicInTrait();
  }
}
class A {
  use TraitFoo;
  public function testDoSomething() {
    return $this->doSomething();
  }
  protected function doSomething() {
    return 'doSomething';
  }
  protected function doSomethingInTrait() {
    return 'doSomethingInTrait';
  }
  public function doSomethingPublicInTrait() {
    return 'doSomethingPublicInTrait';
  }
}

<<__EntryPoint>>
function main_2103() {
$a = new A();
echo $a->testDoSomething()."
";
echo $a->testDoSomethingInTrait()."
";
echo $a->testDoSomethingPublicInTrait()."
";
}
