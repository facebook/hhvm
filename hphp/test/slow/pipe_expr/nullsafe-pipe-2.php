<?hh

<<__EntryPoint>>
function main() :mixed{
vec[1, 2, 3] |?> array_map($x ==> $x + 1, $$) |?> var_dump($$);
null |?> array_map($x ==> $x + 1, $$) |> var_dump($$);
}
