<?hh
class BadVarExample {
  public function testBadVarExample() {
    $this->thisFunctionShouldExist();
  }

  var $foo;

  public function thisFunctionShouldExist() {}
}
