<?hh
// Test runtime behavior of shape field punning syntax.
// shape($foo) is shorthand for shape('foo' => $foo)

<<file:__EnableUnstableFeatures('shape_field_punning')>>

<<__EntryPoint>>
function main(): void {
  echo "=== Basic punning ===\n";
  test_basic_punning();

  echo "\n=== Mixed syntax ===\n";
  test_mixed_syntax();

  echo "\n=== Multiple types ===\n";
  test_multiple_types();

  echo "\n=== Position flexibility ===\n";
  test_position_flexibility();

  echo "\n=== Variable name edge cases ===\n";
  test_variable_names();

  echo "\n=== Shape operations ===\n";
  test_shape_operations();
}

function test_basic_punning(): void {
  $foo = 42;
  $bar = 'hello';

  // Punned fields use variable name (without $) as key
  $s = shape($foo, $bar);

  var_dump($s['foo']);
  var_dump($s['bar']);
  var_dump($s);
}

function test_mixed_syntax(): void {
  $punned = 100;

  // Can mix punned and explicit syntax freely
  $s = shape($punned, 'explicit' => 200);

  var_dump($s['punned']);
  var_dump($s['explicit']);
  var_dump($s);
}

function test_multiple_types(): void {
  $int_val = 42;
  $string_val = 'text';
  $bool_val = true;
  $float_val = 3.14;
  $null_val = null;
  $vec_val = vec[1, 2, 3];

  $s = shape($int_val, $string_val, $bool_val, $float_val, $null_val, $vec_val);

  var_dump($s['int_val']);
  var_dump($s['string_val']);
  var_dump($s['bool_val']);
  var_dump($s['float_val']);
  var_dump($s['null_val']);
  var_dump($s['vec_val']);
}

function test_position_flexibility(): void {
  $first = 1;
  $middle = 2;
  $last = 3;

  // Punned at beginning
  $s1 = shape($first, 'explicit' => 99);
  var_dump($s1);

  // Punned in middle
  $s2 = shape('a' => 10, $middle, 'b' => 20);
  var_dump($s2);

  // Punned at end
  $s3 = shape('x' => 50, $last);
  var_dump($s3);

  // All punned
  $s4 = shape($first, $middle, $last);
  var_dump($s4);
}

function test_variable_names(): void {
  // Single character
  $a = 1;
  $s1 = shape($a);
  var_dump($s1['a']);

  // Alphanumeric
  $var123 = 2;
  $s2 = shape($var123);
  var_dump($s2['var123']);

  // Underscores
  $_private = 3;
  $__double = 4;
  $_under_score_ = 5;
  $s3 = shape($_private, $__double, $_under_score_);
  var_dump($s3['_private']);
  var_dump($s3['__double']);
  var_dump($s3['_under_score_']);

  // Long name
  $thisIsAVeryLongVariableName = 6;
  $s4 = shape($thisIsAVeryLongVariableName);
  var_dump($s4['thisIsAVeryLongVariableName']);
}

function test_shape_operations(): void {
  $name = 'Alice';
  $age = 30;

  $person = shape($name, $age);

  // Shapes::idx works with punned fields
  var_dump(Shapes::idx($person, 'name'));
  var_dump(Shapes::idx($person, 'age'));

  // Shapes::toDict works
  $as_dict = Shapes::toDict($person);
  var_dump($as_dict['name']);
  var_dump($as_dict['age']);
}
