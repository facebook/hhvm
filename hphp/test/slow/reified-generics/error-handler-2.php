<?hh

class Foo<reify T> {}

function f(<<__Soft>> Foo<int> $a) :mixed{
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

f(new Foo<int>);
f(new Foo<string>);
}
