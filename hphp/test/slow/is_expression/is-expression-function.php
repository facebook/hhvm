<?hh

function is_function(mixed $x): void {
  if ($x is (function(): int)) {
    echo "unreached\n";
  }
}


<<__EntryPoint>>
function main_is_expression_function() {
is_function(() ==> 1);
}
