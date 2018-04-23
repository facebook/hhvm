<?hh

function is_array_(mixed $x): void {
  if ($x is array) {
    echo "array\n";
  } else {
    echo "not array\n";
  }
}

is_array_(array());
