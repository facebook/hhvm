<?hh

function is_darray_(mixed $x): void {
  if ($x is darray) {
    echo "darray\n";
  } else {
    echo "not darray\n";
  }
}


<<__EntryPoint>>
function main_is_expression_darray() :mixed{
is_darray_(vec[]);
is_darray_(dict[]);
is_darray_(keyset[]);
is_darray_(HH\array_mark_legacy(vec[]));
is_darray_(HH\array_mark_legacy(dict[]));
}
