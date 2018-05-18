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

is_dict_(darray[]);
is_dict_(varray[]);
is_vec_(varray[]);
is_vec_(darray[]);
