<?hh

function is_void($x): void {
  if ($x is void) {
    echo "void\n";
  } else {
    echo "not void\n";
  }
}

function return_void(): void {}


<<__EntryPoint>>
function main_is_expression_void() {
is_void(return_void());
is_void(null);
is_void(-1);
is_void(false);
is_void(1.5);
is_void('foo');
is_void(STDIN);
is_void(new stdClass());
is_void(tuple(1, 2, 3));
is_void(shape('a' => 1, 'b' => 2));
}
