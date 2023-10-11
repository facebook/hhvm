<?hh

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
  test_signature_help_params1("hi", "there");
  test_signature_help_params2("hi", "there");
  test_signature_help_params3("hi", "there");
  test_signature_help_highlight("hi", "there", "bootcamp");
}

/* comment describing the method
  @param $param1 info1
  @param param2 info2
*/
function test_signature_help_params1(string $param1, string $param2): void {}

/* comment describing the method
  @param $param1 info1
*/
function test_signature_help_params2(string $param1, string $param2): void {}

/*
 * @param $param1 info1
 *                for param1
 * @param $param2   info2
 * @return the string
 *         'hack'
*/
function test_signature_help_params3(string $param1, string $param2): string {
  return 'hack';
}

function test_signature_help_highlight(
  string $param1,
  string $param2,
  string $param3,
): string {
  return 'hack';
}
