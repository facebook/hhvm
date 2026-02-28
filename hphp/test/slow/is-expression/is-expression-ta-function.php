<?hh

type Tfunction = (function(): int);

function is_function(mixed $x): void {
  if ($x is Tfunction) {
    echo "unreached\n";
  }
}


<<__EntryPoint>>
function main_is_expression_ta_function() :mixed{
is_function(() ==> 1);
}
