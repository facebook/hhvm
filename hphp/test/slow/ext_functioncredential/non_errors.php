<?hh

function print_cred_info($cred) {
  $info = $cred->__debugInfo();
  $class_name = idx($info, 'class_name');
  $function_name = idx($info, 'function_name');
  $file_name = idx($info, 'file_name');
  echo "class=".$class_name.PHP_EOL;
  echo "function=".$function_name.PHP_EOL;
  $e = explode('/', $file_name); echo "file=".end(inout $e).PHP_EOL;
}

function test_function() {
  echo "--test_function--".PHP_EOL;
  print_cred_info(__FUNCTION_CREDENTIAL__);
  echo PHP_EOL;
}

trait TestTrait {
  public function test_trait_method() {
    echo "--test_trait_method--".PHP_EOL;
    print_cred_info(__FUNCTION_CREDENTIAL__);
    echo PHP_EOL;
  }
}

class TestClass {
  use TestTrait;

  public static function test_static_method() {
    echo "--test_static_method--".PHP_EOL;
    print_cred_info(__FUNCTION_CREDENTIAL__);
    echo PHP_EOL;
  }

  public function test_method() {
    echo "--test_method--".PHP_EOL;
    print_cred_info(__FUNCTION_CREDENTIAL__);
    echo PHP_EOL;
  }

  public static function test_class_lambda() {
    return () ==> {
      echo "--test_class_lambda--".PHP_EOL;
      print_cred_info(__FUNCTION_CREDENTIAL__);
      echo PHP_EOL;
    };
  }
}
<<__EntryPoint>> function main(): void {
$test_lambda = () ==> {
  echo "--test_lambda--".PHP_EOL;
  print_cred_info(__FUNCTION_CREDENTIAL__);
  echo PHP_EOL;
};

$test_long_lambda = function() {
  echo "--test_long_lambda--".PHP_EOL;
  print_cred_info(__FUNCTION_CREDENTIAL__);
  echo PHP_EOL;
};

test_function();
TestClass::test_static_method();
new TestClass()->test_method();
new TestClass()->test_trait_method();
$test_lambda();
TestClass::test_class_lambda()();
$test_long_lambda();
}
