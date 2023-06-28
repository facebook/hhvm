<?hh

function is_soft(mixed $x): void {
  if ($x is <<__Soft>> int) {
    echo "unreached\n";
  }
}


<<__EntryPoint>>
function main_is_expression_soft() :mixed{
is_function(1);
}
