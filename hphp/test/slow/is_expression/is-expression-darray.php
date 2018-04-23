<?hh

function is_darray_(mixed $x): void {
  if ($x is darray) {
    echo "darray\n";
  } else {
    echo "not darray\n";
  }
}

is_darray_(darray[]);
