<?hh

class C {}

function is_C(mixed $x): void {
  if ($x is C) {
    echo "C\n";
  } else {
    echo "not C\n";
  }
}


<<__EntryPoint>>
function main(): void {
  is_C(null);
  is_C(-1);
  is_C(false);
  is_C(1.5);
  is_C('foo');
  is_C(STDIN);
  is_C(new stdClass());
  is_C(new C());
  is_C(tuple(1, 2, 3));
  is_C(shape('a' => 1, 'b' => 2));
}
