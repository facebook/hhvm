<?hh

function is_varray_(mixed $x): void {
  if ($x is varray) {
    echo "varray\n";
  } else {
    echo "not varray\n";
  }
}


<<__EntryPoint>>
function main_is_expression_varray() {
is_varray_(varray[]);
}
