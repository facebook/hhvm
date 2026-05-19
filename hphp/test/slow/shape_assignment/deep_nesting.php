<?hh

<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>;

function test_three_level_shape(): void {
  echo "test_three_level_shape\n";
  $s = shape('a' => shape('b' => shape('c' => 42)));
  shape('a' => shape('b' => shape('c' => $val))) = $s;
  echo "val="; var_dump($val);
}

function test_mixed_nesting(): void {
  echo "test_mixed_nesting\n";
  $s = shape('items' => tuple(shape('name' => 'Alice'), 100));
  shape('items' => tuple(shape('name' => $name), $count)) = $s;
  echo "name="; var_dump($name);
  echo "count="; var_dump($count);
}

function test_tuple_three_level(): void {
  echo "test_tuple_three_level\n";
  $t = tuple(tuple(tuple(99)));
  tuple(tuple(tuple($val))) = $t;
  echo "val="; var_dump($val);
}

<<__EntryPoint>>
function main(): void {
  test_three_level_shape();
  test_mixed_nesting();
  test_tuple_three_level();
}
