<?hh

function is_darray_(mixed $x): void {
  if ($x is darray) {
    echo "darray\n";
  } else {
    echo "not darray\n";
  }
}


<<__EntryPoint>>
function main_is_expression_darray() {
is_darray_(darray[]);
}
