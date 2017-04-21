<?hh

$foo = "Hello!";
array(1, 2, 3)
  |> array_map($x ==> $x + 1, $$)
  |> array_merge(
    array(50, 60, 70)
      |> array_map($x ==> $x * 2, $$)
      |> array_filter($$, $x ==> $x != 100),
    $$)
  |> var_dump($$);

var_dump($foo);
