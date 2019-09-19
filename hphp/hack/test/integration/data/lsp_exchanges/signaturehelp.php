<?hh // strict

/** Global function with doc block */
function global_function(string $s, int $x): void {}

class MyClass {
  /** Constructor with doc block */
  public function __construct() {}
  /** Static method with doc block */
  public static function staticMethod(string $z): void {}
  /** Instance method with doc block */
  public function instanceMethod(int $x1, int $x2): void {}
}

function test_signature_help(): void
{
  $x = new MyClass();
  $x->instanceMethod(1,2);
  MyClass::staticMethod("hi");
  global_function("hi", 1);
  Herp\aliased_global_func("hi");
}
