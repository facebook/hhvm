<?hh

function is_array_(mixed $x): void {
  if ($x is array) {
    echo "array\n";
  } else {
    echo "not array\n";
  }
}


<<__EntryPoint>>
function main_is_expression_array() {
is_array_(varray[]);
}
