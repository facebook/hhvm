<?hh

class C<reify T> {}

function f(mixed $x): <<__Soft>> C<int> {
  return $x;
}
<<__EntryPoint>> function main(): void {
set_error_handler(
  (int $errno,
  string $errstr,
  string $errfile,
  int $errline,
  darray $errcontext
  ) ==> {
    echo "ERROR: "; var_dump($errstr);
    return true;
  }
);

f(new C<int>());
f(new C<string>());
}
