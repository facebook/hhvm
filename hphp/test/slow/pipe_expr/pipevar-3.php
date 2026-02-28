<?hh

function main($bar) :mixed{
  $foo = "Hello!";
  vec[1, 2, 3]
    |> array_map($x ==> $x + 1, $$)
    |> array_merge(
      vec[50, 60, 70]
        |> array_map($x ==> $x * 2, $$)
        |> array_filter($$, $x ==> $x != 100),
      $$)
    |> var_dump($$);

  var_dump($foo);
  var_dump($bar);
}


<<__EntryPoint>>
function main_pipevar_3() :mixed{
main("Goodbye");
}
