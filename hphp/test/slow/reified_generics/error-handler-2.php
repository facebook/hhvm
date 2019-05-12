<?hh

class Foo<reify T> {}

function f(@Foo<int> $a) {
  return $a;
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
    return true;
  }
);

f(new Foo<int>);
f(new Foo<string>);
}
