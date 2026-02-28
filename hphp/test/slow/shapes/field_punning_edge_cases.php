<?hh
// Test edge cases: trailing commas and nested shapes with punning

<<file:__EnableUnstableFeatures('shape_field_punning')>>

<<__EntryPoint>>
function main(): void {
  echo "=== Trailing commas ===\n";
  test_trailing_commas();

  echo "\n=== Nested shapes ===\n";
  test_nested_shapes();

  echo "\n=== Empty and single field ===\n";
  test_empty_and_single();
}

function test_trailing_commas(): void {
  $x = 1;
  $y = 2;

  // Trailing comma with punned fields
  $s = shape($x, $y,);

  var_dump($s['x']);
  var_dump($s['y']);
}

function test_nested_shapes(): void {
  $inner = shape('value' => 42);
  $name = 'outer';

  // Punned field containing a shape
  $outer = shape($inner, $name);

  var_dump($outer['inner']['value']);
  var_dump($outer['name']);

  // Nested punning at multiple levels
  $a = 1;
  $b = 2;
  $nested = shape($a, 'child' => shape($b));

  var_dump($nested['a']);
  var_dump($nested['child']['b']);
}

function test_empty_and_single(): void {
  // Empty shape still works
  $empty = shape();
  var_dump($empty);

  // Single punned field
  $only = 'solo';
  $single = shape($only);
  var_dump($single);
  var_dump($single['only']);
}
