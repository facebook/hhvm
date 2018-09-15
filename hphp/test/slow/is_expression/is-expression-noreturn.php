<?hh

function is_noreturn($x): void {
  if ($x is noreturn) {
    echo "noreturn\n";
  } else {
    echo "not noreturn\n";
  }
}


<<__EntryPoint>>
function main_is_expression_noreturn() {
is_noreturn(null);
is_noreturn(-1);
is_noreturn(false);
is_noreturn(1.5);
is_noreturn('foo');
is_noreturn(STDIN);
is_noreturn(new stdClass());
is_noreturn(tuple(1, 2, 3));
is_noreturn(shape('a' => 1, 'b' => 2));
}
