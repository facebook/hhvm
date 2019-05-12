<?hh

class C<reify T>{}

function f(C<int> $c) {}
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

$c = new C<string>();
f($c);
}
