<?hh

function is_soft(mixed $x): void {
  if ($x is @int) {
    echo "unreached\n";
  }
}


<<__EntryPoint>>
function main_is_expression_soft() {
is_function(1);
}
