<?hh

function top_level_func(): FunctionCredential {
  return __FUNCTION_CREDENTIAL__;
}

<<__Memoize>>
function memoized_func(): FunctionCredential {
  return __FUNCTION_CREDENTIAL__;
}

class TestClass {
  public function instance_method(): FunctionCredential {
    return __FUNCTION_CREDENTIAL__;
  }

  public static function static_method(): FunctionCredential {
    return __FUNCTION_CREDENTIAL__;
  }

  <<__Memoize>>
  public function memoized_method(): FunctionCredential {
    return __FUNCTION_CREDENTIAL__;
  }

  <<__Memoize>>
  public static function memoized_static_method(): FunctionCredential {
    return __FUNCTION_CREDENTIAL__;
  }
}

function assert_eq(string $label, string $expected, string $actual): void {
  if ($expected === $actual) {
    echo "PASS: ".$label." = '".$expected."'\n";
  } else {
    echo "FAIL: ".$label.": expected '".$expected."', got '".$actual."'\n";
  }
}

<<__EntryPoint>>
function main(): void {
  $inst = new TestClass();

  assert_eq(
    'top-level function',
    'top_level_func',
    top_level_func()->getMethodOrFunctionName(),
  );
  assert_eq(
    'memoized top-level includes $memoize_impl',
    'memoized_func$memoize_impl',
    memoized_func()->getMethodOrFunctionName(),
  );
  assert_eq(
    'instance method',
    'TestClass::instance_method',
    $inst->instance_method()->getMethodOrFunctionName(),
  );
  assert_eq(
    'static method',
    'TestClass::static_method',
    TestClass::static_method()->getMethodOrFunctionName(),
  );
  assert_eq(
    'memoized instance method includes $memoize_impl',
    'TestClass::memoized_method$memoize_impl',
    $inst->memoized_method()->getMethodOrFunctionName(),
  );
  assert_eq(
    'memoized static method includes $memoize_impl',
    'TestClass::memoized_static_method$memoize_impl',
    TestClass::memoized_static_method()->getMethodOrFunctionName(),
  );
}
