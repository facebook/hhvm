<?hh

function is_dict_(mixed $x): void {
  if ($x is dict) {
    echo "dict\n";
  } else {
    echo "not dict\n";
  }
}

function is_vec_(mixed $x): void {
  if ($x is vec) {
    echo "vec\n";
  } else {
    echo "not vec\n";
  }
}


<<__EntryPoint>>
function main_is_expression_hack_array_compat() :mixed{
is_dict_(dict[]);
is_dict_(vec[]);
is_vec_(vec[]);
is_vec_(dict[]);
}
