<?hh

<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>;

function test_shape_ellipsis(): void {
  echo "test_shape_ellipsis\n";
  $s = shape('x' => 1, 'y' => 2, 'z' => 3);
  shape('x' => $x, ...) = $s;
  echo "x="; var_dump($x);
}

function test_tuple_ellipsis(): void {
  echo "test_tuple_ellipsis\n";
  $t = tuple(1, 'hello', true);
  tuple($a, ...) = $t;
  echo "a="; var_dump($a);
}

function test_nested_ellipsis(): void {
  echo "test_nested_ellipsis\n";
  $s = shape('outer' => shape('a' => 1, 'b' => 2, 'c' => 3));
  shape('outer' => shape('a' => $a, ...)) = $s;
  echo "a="; var_dump($a);
}

function test_empty_shape_ellipsis(): void {
  echo "test_empty_shape_ellipsis\n";
  $s = shape('x' => 1, 'y' => 2);
  shape(...) = $s;
  echo "ok\n";
}

function test_foreach_shape_ellipsis(): void {
  echo "test_foreach_shape_ellipsis\n";
  $items = vec[
    shape('x' => 1, 'y' => 10, 'z' => 100),
    shape('x' => 2, 'y' => 20, 'z' => 200),
  ];
  foreach ($items as shape('x' => $x, ...)) {
    echo "x="; var_dump($x);
  }
}

function test_foreach_tuple_ellipsis(): void {
  echo "test_foreach_tuple_ellipsis\n";
  $items = vec[
    tuple(1, 'a', true),
    tuple(2, 'b', false),
  ];
  foreach ($items as tuple($n, ...)) {
    echo "n="; var_dump($n);
  }
}

<<__EntryPoint>>
function main(): void {
  test_shape_ellipsis();
  test_tuple_ellipsis();
  test_nested_ellipsis();
  test_empty_shape_ellipsis();
  test_foreach_shape_ellipsis();
  test_foreach_tuple_ellipsis();
}
