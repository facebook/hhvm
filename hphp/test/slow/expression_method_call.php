<?hh


<<__EntryPoint>>
function main_expression_method_call() :mixed{
var_dump(
  (true ? Vector{1} : Vector{2})->add(3)
);
var_dump(
  (false ?: Map{1 => 'a'})->add(Pair{2, 'b'})
);
var_dump(
  (Set{1} |> $$->add(2))->add(4)
);
}
