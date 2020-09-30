<?hh

type FunctionCrededentialExpectations = shape(
  'class_name' => ?string,
  'function_name' => string,
);

function run_test(
  string $test_name,
  FunctionCredential $cred,
  FunctionCrededentialExpectations $expectations,
): void {
  $errors = vec[];

  $class_name = $cred->getClassName();
  $function_name = $cred->getFunctionName();
  $file_name_pieces = $cred->getFilename()
    |> explode('/', $$);
  $file_name_stripped = $file_name_pieces[count($file_name_pieces) - 1];

  if ($class_name !== $expectations['class_name']) {
    $errors[] = "Expected Class Name ".
      $expectations['class_name'].
      " does not match actual Class Name ".
      $class_name.
      " from ".
      $meth;
  }

  if ($function_name !== $expectations['function_name']) {
    $errors[] = "Expected Function Name ".
      $expectations['function_name'].
      " does not match actual Function Name ".
      $function_name.
      " from ".
      $meth;
  }

  if ($file_name_stripped !== 'non_errors.php') {
      $errors[] = "Expected File Name non_errors.php ".
      "does not match actual File Name ".
      $file_name_stripped.
      " from ".
      $meth;
  }

  if ($errors) {
    echo "Errors in `".$test_name."`:".PHP_EOL;
    foreach ($errors as $error) {
      echo $error.PHP_EOL;
    }
  }
}

function test_function(): void {
  run_test(
    'test_function',
    __FUNCTION_CREDENTIAL__,
    shape('class_name' => null, 'function_name' => 'test_function'),
  );
}

trait TestTrait {
  public function test_trait_method(): void {
    run_test(
      'test_trait_method',
      __FUNCTION_CREDENTIAL__, shape(
      'class_name' => 'TestClass',
      'function_name' => 'test_trait_method',
    ));
  }

  public static function test_static_trait_method(): void {
    run_test(
      'test_static_trait_method',
      __FUNCTION_CREDENTIAL__, shape(
      'class_name' => 'TestClass',
      'function_name' => 'test_static_trait_method',
    ));
  }
}

abstract class BaseClass {
  public function test_inherited_method(): void {
    run_test('test_inherited_method', __FUNCTION_CREDENTIAL__, shape(
      'class_name' => 'BaseClass',
      'function_name' => 'test_inherited_method',
    ));
  }

  public static function test_static_inherited_method(): void {
    run_test('test_static_inherited_method', __FUNCTION_CREDENTIAL__, shape(
      'class_name' => 'BaseClass',
      'function_name' => 'test_static_inherited_method',
    ));
  }
}

class TestClass extends BaseClass {
  use TestTrait;

  public function __construct() {
    run_test(
      'test_constructor',
        __FUNCTION_CREDENTIAL__,
        shape('class_name' => 'TestClass', 'function_name' => '__construct'),
    );
  }

  public static function test_static_method(): void {
    run_test('test_static_method', __FUNCTION_CREDENTIAL__, shape(
      'class_name' => 'TestClass',
      'function_name' => 'test_static_method',
    ));
  }

  public function test_method(): void {
    run_test(
      'test_method',
      __FUNCTION_CREDENTIAL__,
      shape('class_name' => 'TestClass', 'function_name' => 'test_method'),
    );
  }

  public static function test_class_lambda(): (function(): void) {
    return () ==> {
      run_test(
        'test_class_lambda',
        __FUNCTION_CREDENTIAL__,
        shape('class_name' => 'TestClass', 'function_name' => '__invoke'),
      );
    };
  }
}

<<__EntryPoint>>
function main(): void {
  $test_lambda = () ==> {
    run_test(
      'test_lambda',
      __FUNCTION_CREDENTIAL__,
      shape('class_name' => null, 'function_name' => '__invoke'),
    );
  };

  $test_long_lambda = function(): void {
    run_test(
      'test_long_lambda',
      __FUNCTION_CREDENTIAL__,
      shape('class_name' => null, 'function_name' => '__invoke'),
    );
  };

  test_function();
  TestClass::test_static_method();
  TestClass::test_static_trait_method();
  TestClass::test_static_inherited_method();
  $inst = new TestClass();
  $inst->test_method();
  $inst->test_trait_method();
  $inst->test_inherited_method();
  $test_lambda();
  TestClass::test_class_lambda()();
  $test_long_lambda();

  echo 'Test Complete.'.PHP_EOL;
}
