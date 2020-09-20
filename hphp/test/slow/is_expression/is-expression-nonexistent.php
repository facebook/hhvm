<?hh

function is_nonexistent(mixed $x): void {
  if ($x is BlerpityBlerp) {
    echo "unreached\n";
  }
}


<<__EntryPoint>>
function main_is_expression_nonexistent() {
is_nonexistent(new stdClass());
}
