<?hh

function is_nonexistent(mixed $x): void {
  if ($x is BlerpityBlerp) {
    echo "unreached\n";
  } else {
    echo "not BlerpityBlerp\n";
  }
}

function fail(
  string $type,
  string $name,
  mixed $err = null,
): noreturn {
  throw new Exception('Unable to load class');
}

<<__EntryPoint>>
function main(): void {
  is_nonexistent(new stdClass());
}
