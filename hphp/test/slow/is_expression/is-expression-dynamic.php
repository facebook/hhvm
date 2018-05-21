<?hh

function is_dynamic($x): void {
  if ($x is dynamic) {
    echo "dynamic\n";
  } else {
    echo "not dynamic\n";
  }
}

is_dynamic(null);
is_dynamic(-1);
is_dynamic(false);
is_dynamic(1.5);
is_dynamic('foo');
is_dynamic(STDIN);
is_dynamic(new stdClass());
is_dynamic(tuple(1, 2, 3));
is_dynamic(shape('a' => 1, 'b' => 2));
