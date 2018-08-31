<?hh

type Tnonexistent = shape('f' => BlerpityBlerp);

function is_nonexistent(mixed $x): void {
  if ($x is Tnonexistent) {
    echo "unreached\n";
  }
}


<<__EntryPoint>>
function main_is_expression_ta_nonexistent() {
is_nonexistent(new stdClass());
}
