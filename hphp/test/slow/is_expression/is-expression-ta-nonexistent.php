<?hh

type Tnonexistent = shape('f' => BlerpityBlerp);

function is_nonexistent(mixed $x): void {
  if ($x is Tnonexistent) {
    echo "unreached\n";
  }
}

is_nonexistent(new stdClass());
