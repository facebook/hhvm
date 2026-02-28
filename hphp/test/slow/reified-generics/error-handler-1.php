<?hh

class Foo {}

function f(<<__Soft>> Foo $a) :mixed{
  return $a;
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

f(dict[]);
}
