<?hh

function is_T(mixed $x): void {
  if ($x is parent::T) {
    echo "T\n";
  } else {
    echo "not T\n";
  }
}


<<__EntryPoint>>
function main_is_expression_tc_parent_nonclass() {
is_T(new stdClass());
}
