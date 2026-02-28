<?hh

trait TraitFoo {
  public function getStringThroughProtectedMethod() :mixed{
    return $this->protectedMethod();
  }
  protected function protectedMethod() :mixed{
    return 'fallback protected method';
  }
  public function getStringThroughPrivateMethod() :mixed{
    return $this->privateMethod();
  }
  private function privateMethod() :mixed{
    return 'fallback private method';
  }
}
class A {
  use TraitFoo;
  protected function protectedMethod() :mixed{
    return 'in a protectedMethod';
  }
  private function privateMethod() :mixed{
    return 'in a privateMethod';
  }
}

<<__EntryPoint>>
function main_2065() :mixed{
$a = new A();
echo $a->getStringThroughProtectedMethod()."\n";
echo $a->getStringThroughPrivateMethod()."\n";
}
