<?hh

type Tfunction = shape('a' => (function(): int));

function is_function(mixed $x): void {
  if ($x is Tfunction) {
    echo "unreached\n";
  }
}


<<__EntryPoint>>
function main_is_expression_ta_shape_function() :mixed{
is_function(shape('a' => () ==> 1));
}
