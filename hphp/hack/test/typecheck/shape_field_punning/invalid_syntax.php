<?hh
<<file:__EnableUnstableFeatures('shape_field_punning')>>

class Foo {
  public int $prop = 1;
}

function test_property_access_without_arrow(): void {
  $obj = new Foo();
  // Property access without arrow should be a parse error
  $s = shape($obj->prop);
}

function test_array_access_without_arrow(): void {
  $arr = vec[1, 2, 3];
  // Array access without arrow should be a parse error
  $s = shape($arr[0]);
}

function test_function_call_without_arrow(): void {
  // Function call without arrow should be a parse error
  $s = shape(get_value());
}

function test_literal_without_arrow(): void {
  // Literal without arrow should be a parse error
  $s = shape(42);
}

function test_string_literal_without_arrow(): void {
  // String literal without arrow should be a parse error
  $s = shape('hello');
}

function get_value(): int {
  return 1;
}
