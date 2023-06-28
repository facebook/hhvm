<?hh


<<__EntryPoint>>
function main_pipevar_1() :mixed{
varray[1, 2, 3] |> array_map($x ==> $x + 1, $$) |> var_dump($$);
}
