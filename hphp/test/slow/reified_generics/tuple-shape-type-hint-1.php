<?hh

function handler($errno, $errstr, $errfile, $errline) {
  throw new Exception($errstr);
}

function f<reify T>(T $x) {
  var_dump($x is T);
}

class C<reify T> {}

function g<reify T>(C<T> $x) {
  var_dump($x is C<T>);
}

<<__EntryPoint>>
function main() {
  set_error_handler(fun("handler"));
  // pass type hint but fail is
  f<shape('a' => int)>(shape());
  // pass type hint and is
  f<shape('a' => int)>(shape('a' => 1));

  // pass type hint but fail is
  f<(int)>(tuple());
  // pass type hint and is
  f<(int)>(tuple(1));

  // fail type hint and is
  try { f<C<shape('a' => int)>>(new C<shape()>()); }
  catch (Exception $e) { var_dump($e->getMessage()); }
  // pass type hint and is
  f<C<shape('a' => int)>>(new C<shape('a' => int)>());

  // pass type hint and is
  f<C<(int)>>(new C<(int)>());
}
