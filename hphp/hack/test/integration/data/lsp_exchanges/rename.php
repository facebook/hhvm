<?hh  //strict
// comment
function a_rename(): int {
  return b_rename();
}

function b_rename(): int {
  return 42;
}

class TestClass {
  public function __construct(int $i) {}

  public function test_method(): int {
    return 1;
  }
}

function test_rename(): void {
  $test_class = new TestClass(1);
  $test_class->test_method(); $test_class->test_method();
}
