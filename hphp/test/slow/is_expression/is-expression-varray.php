<?hh

function is_varray_(mixed $x): void {
  if ($x is varray) {
    echo "varray\n";
  } else {
    echo "not varray\n";
  }
}

is_varray_(varray[]);
