<?hh

class C<reify T>{}

function f(C<int> $c) {}

set_error_handler(
  (int $errno,
  string $errstr,
  string $errfile,
  int $errline,
  array $errcontext
  ) ==> {
    echo "ERROR: "; var_dump($errstr);
    throw new Exception();
  }
);

$c = new C<string>();
f($c);
