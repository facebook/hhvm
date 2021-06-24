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
is_varray_(vec[]);
is_varray_(dict[]);
is_varray_(keyset[]);
is_varray_(HH\array_mark_legacy(vec[]));
is_varray_(HH\array_mark_legacy(dict[]));
}
