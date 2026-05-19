<?hh

<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>;

function test_basic_tuple(): void {
  echo "test_basic_tuple\n";
  $t = tuple(1, 'hello');
  tuple($a, $b) = $t;
  echo "a="; var_dump($a);
  echo "b="; var_dump($b);
}

function test_three_elements(): void {
  echo "test_three_elements\n";
  $t = tuple(10, 20, 30);
  tuple($a, $b, $c) = $t;
  echo "a="; var_dump($a);
  echo "b="; var_dump($b);
  echo "c="; var_dump($c);
}

function test_tuple_nesting(): void {
  echo "test_tuple_nesting\n";
  $t = tuple(tuple(1, 2), tuple(3, 4));
  tuple(tuple($a, $b), tuple($c, $d)) = $t;
  echo "a="; var_dump($a);
  echo "b="; var_dump($b);
  echo "c="; var_dump($c);
  echo "d="; var_dump($d);
}

function test_tuple_foreach(): void {
  echo "test_tuple_foreach\n";
  $items = vec[tuple(1, 'a'), tuple(2, 'b')];
  foreach ($items as tuple($n, $s)) {
    echo "n="; var_dump($n);
    echo "s="; var_dump($s);
  }
}

<<__EntryPoint>>
function main(): void {
  test_basic_tuple();
  test_three_elements();
  test_tuple_nesting();
  test_tuple_foreach();
}
