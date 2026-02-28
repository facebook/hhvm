<?hh

trait TraitFoo {
  public function testDoSomethingInTrait() :mixed{
    return $this->doSomethingInTrait();
  }
  public function testDoSomethingPublicInTrait() :mixed{
    return $this->doSomethingPublicInTrait();
  }
}
class A {
  use TraitFoo;
  public function testDoSomething() :mixed{
    return $this->doSomething();
  }
  protected function doSomething() :mixed{
    return 'doSomething';
  }
  protected function doSomethingInTrait() :mixed{
    return 'doSomethingInTrait';
  }
  public function doSomethingPublicInTrait() :mixed{
    return 'doSomethingPublicInTrait';
  }
}

<<__EntryPoint>>
function main_2103() :mixed{
$a = new A();
echo $a->testDoSomething()."
";
echo $a->testDoSomethingInTrait()."
";
echo $a->testDoSomethingPublicInTrait()."
";
}
