<?hh

array(1, 2, 3) |> array_map($x ==> $x + 1, $$) |> var_dump($$);
