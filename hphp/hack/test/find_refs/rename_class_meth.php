<?hh

class RenameClassMethClass {

  public function instance_test_method(): int {
    return 2;
  }

  // Find-refs should be able to find all these uses of instance_test_method
  // When the method is wrapped in apostrophes, the capture should be only
  // the method name
  public function other_method(): int {
    $this->instance_test_method();
    meth_caller(RenameClassMethClass::class, 'instance_test_method');
    return 0;
  }
}
