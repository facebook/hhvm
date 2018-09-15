<?hh

function is_varray_or_darray_(mixed $x): void {
  if ($x is varray_or_darray) {
    echo "varray_or_darray\n";
  } else {
    echo "not varray_or_darray\n";
  }
}


<<__EntryPoint>>
function main_is_expression_varray_or_darray() {
is_varray_or_darray_(varray[]);
}
