<?hh
<<file:__EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

// Tests for named parameters in methods

class TestClass {
  public function method_with_named_params(int $a, named int $b, named int $c): void {}

  public static function static_method_with_named_params(int $a, named int $b, named int $c): void {}
}

function test_methods(): void {
  $obj = new TestClass();

  $obj->method_with_named_params(1, b=2, c=3);

  TestClass::static_method_with_named_params(1, b=2, c=3);

  // Error: Missing required named parameter in instance method
  $obj->method_with_named_params(1, b=2);

  // Error: Missing required named parameter in static method
  TestClass::static_method_with_named_params(1, c=3);

  // Error: Unexpected named parameter in instance method
  $obj->method_with_named_params(1, b=2, c=3, d=4);

  // Error: Unexpected named parameter in static method
  TestClass::static_method_with_named_params(1, b=2, c=3, d=4);
}
