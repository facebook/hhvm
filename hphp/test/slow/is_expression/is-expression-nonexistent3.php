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
  HH\autoload_set_paths(
    dict[
      'class' => dict[],
      'constant' => dict[],
      'function' => dict[],
      'type' => dict[],
      'failure' => fun('fail'),
    ],
    __DIR__.'/',
  );
  is_nonexistent(new stdClass());
}
