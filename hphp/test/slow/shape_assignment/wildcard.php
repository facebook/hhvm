<?hh

<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>;

function test_wildcard_shape(): void {
  echo "test_wildcard_shape\n";
  $s = shape('x' => 1, 'y' => 2);
  shape('x' => $x, 'y' => _) = $s;
  echo "x="; var_dump($x);
}

function test_wildcard_tuple(): void {
  echo "test_wildcard_tuple\n";
  $t = tuple(1, 'hello', true);
  tuple(_, $b, _) = $t;
  echo "b="; var_dump($b);
}

function test_all_wildcards(): void {
  echo "test_all_wildcards\n";
  $s = shape('x' => 1, 'y' => 2);
  shape('x' => _, 'y' => _) = $s;
  echo "ok\n";
}

function test_foreach_wildcard(): void {
  echo "test_foreach_wildcard\n";
  $items = vec[
    shape('x' => 1, 'y' => 10),
    shape('x' => 2, 'y' => 20),
  ];
  foreach ($items as shape('x' => $x, 'y' => _)) {
    echo "x="; var_dump($x);
  }
}

function test_wildcard_accesses_field(): void {
  echo "test_wildcard_accesses_field\n";
  $s = shape('x' => 1);
  try {
    shape('x' => $x, 'y' => _) = $s;
    echo "no exception\n";
  } catch (\Exception $e) {
    echo "threw\n";
  }
}

<<__EntryPoint>>
function main(): void {
  test_wildcard_shape();
  test_wildcard_tuple();
  test_all_wildcards();
  test_foreach_wildcard();
  test_wildcard_accesses_field();
}
