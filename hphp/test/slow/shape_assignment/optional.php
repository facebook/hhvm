<?hh

<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>;

function test_optional_present(): void {
  echo "test_optional_present\n";
  $s = shape('x' => 1, 'y' => 2);
  shape('x' => $x, ?'y' => $y) = $s;
  echo "x="; var_dump($x);
  echo "y="; var_dump($y);
}

function test_optional_absent(): void {
  echo "test_optional_absent\n";
  $s = shape('x' => 1);
  shape('x' => $x, ?'y' => $y, ...) = $s;
  echo "x="; var_dump($x);
  echo "y="; var_dump($y);
}

<<__EntryPoint>>
function main(): void {
  test_optional_present();
  test_optional_absent();
}
