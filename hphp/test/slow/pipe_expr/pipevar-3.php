<?hh

function main($bar) {
  $foo = "Hello!";
  varray[1, 2, 3]
    |> array_map($x ==> $x + 1, $$)
    |> array_merge(
      varray[50, 60, 70]
        |> array_map($x ==> $x * 2, $$)
        |> array_filter($$, $x ==> $x != 100),
      $$)
    |> var_dump($$);

  var_dump($foo);
  var_dump($bar);
}


<<__EntryPoint>>
function main_pipevar_3() {
main("Goodbye");
}
