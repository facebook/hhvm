<?hh

function is_T(mixed $x): void {
  if ($x is self::T) {
    echo "T\n";
  } else {
    echo "not T\n";
  }
}


<<__EntryPoint>>
function main_is_expression_tc_self_nonclass() :mixed{
is_T(new stdClass());
}
