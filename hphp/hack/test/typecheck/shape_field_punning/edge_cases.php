<?hh
<<file:__EnableUnstableFeatures('shape_field_punning')>>

function test_single_char_var(): void {
  $a = 1;
  $s = shape($a);
  hh_expect<shape('a' => int)>($s);
}

function test_alphanumeric_var(): void {
  $foo123 = 1;
  $s = shape($foo123);
  hh_expect<shape('foo123' => int)>($s);
}

function test_underscore_prefix(): void {
  $_private = 'secret';
  $s = shape($_private);
  hh_expect<shape('_private' => string)>($s);
}

function test_long_variable_name(): void {
  $thisIsAVeryLongVariableNameThatShouldStillWork = 42;
  $s = shape($thisIsAVeryLongVariableNameThatShouldStillWork);
  hh_expect<shape('thisIsAVeryLongVariableNameThatShouldStillWork' => int)>($s);
}

function test_multiple_underscores(): void {
  $__double = 1;
  $_under_score_ = 2;
  $s = shape($__double, $_under_score_);
  hh_expect<shape('__double' => int, '_under_score_' => int)>($s);
}

function test_numeric_suffix(): void {
  $var1 = 1;
  $var2 = 2;
  $var3 = 3;
  $s = shape($var1, $var2, $var3);
  hh_expect<shape('var1' => int, 'var2' => int, 'var3' => int)>($s);
}
