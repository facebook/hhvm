<?hh

function none_member_function($num): int {
  $obj = AnotherTestClass::createInstance();
  $obj->publicMethod("another test");
  return $num + 1;
}

class AnotherTestClass {

  private string $field;
  public function __construct(string $input) {
    $this->field = $input;
  }

  public function publicMethod($arg): int {
    return 100;
  }

  static public function createInstance(): AnotherTestClass {
    $obj = new AnotherTestClass("test");
    $obj->publicMethod("dummy");

    AnotherTestClass::staticMethod();

    none_member_function(10);
    return $obj;
  }

  static public function staticMethod(): void {}
}
