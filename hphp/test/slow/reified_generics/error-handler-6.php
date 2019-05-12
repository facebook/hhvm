<?hh

class C<reify T> {}

function f(mixed $x): C<int> {
  return $x;
}
<<__EntryPoint>> function main(): void {
set_error_handler(
  (int $errno,
  string $errstr,
  string $errfile,
  int $errline,
  array $errcontext
  ) ==> {
    echo "ERROR: "; var_dump($errstr);
    return false;
  }
);

f(new C<int>());
f(new C<string>());
}
