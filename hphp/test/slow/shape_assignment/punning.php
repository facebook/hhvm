<?hh

<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring', 'shape_field_punning')>>;

function test_basic_punning(): void {
  echo "test_basic_punning\n";
  $s = shape('x' => 1, 'y' => 2);
  shape($x, $y) = $s;
  echo "x="; var_dump($x);
  echo "y="; var_dump($y);
}

function test_mixed_punning(): void {
  echo "test_mixed_punning\n";
  $s = shape('a' => 10, 'b' => 20, 'c' => 30);
  shape($a, 'c' => $cc, $b) = $s;
  echo "a="; var_dump($a);
  echo "b="; var_dump($b);
  echo "cc="; var_dump($cc);
}

function test_punning_with_ellipsis(): void {
  echo "test_punning_with_ellipsis\n";
  $s = shape('x' => 1, 'y' => 2, 'z' => 3);
  shape($x, ...) = $s;
  echo "x="; var_dump($x);
}

function test_optional_punning(): void {
  echo "test_optional_punning\n";
  $s1 = shape('x' => 1, 'y' => 2);
  shape($x, ?$y) = $s1;
  echo "x="; var_dump($x);
  echo "y="; var_dump($y);

  $s2 = shape('a' => 10);
  shape($a, ?$b, ...) = $s2;
  echo "a="; var_dump($a);
  echo "b="; var_dump($b);
}

<<__EntryPoint>>
function main(): void {
  test_basic_punning();
  test_mixed_punning();
  test_punning_with_ellipsis();
  test_optional_punning();
}
