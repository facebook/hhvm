<?hh

<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>;

function test_nested_present(): void {
  echo "test_nested_present\n";
  $p = shape('name' => 'Alice', 'addr' => shape('city' => 'NYC', 'zip' => '10001'));
  shape('name' => $name, ?'addr' => shape('city' => $city, 'zip' => $zip)) = $p;
  echo "name="; var_dump($name);
  echo "city="; var_dump($city);
  echo "zip="; var_dump($zip);
}

function test_nested_absent(): void {
  echo "test_nested_absent\n";
  $p = shape('name' => 'Bob');
  shape('name' => $name, ?'addr' => shape('city' => $city, ...)) = $p;
  echo "name="; var_dump($name);
  echo "city="; var_dump($city);
}

function test_nested_double_optional(): void {
  echo "test_nested_double_optional\n";
  $p = shape('name' => 'Eve');
  shape('name' => $name, ?'addr' => shape(?'zip' => $zip, ...)) = $p;
  echo "name="; var_dump($name);
  echo "zip="; var_dump($zip);
}

<<__EntryPoint>>
function main(): void {
  test_nested_present();
  test_nested_absent();
  test_nested_double_optional();
}
