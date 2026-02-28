<?hh
function handler($errno, $errstr, $errfile, $errline, darray $errcontext) :mixed{
  echo "handler_called\n";
}
<<__EntryPoint>>
function main(): void {
  set_error_handler(handler<>);
  try {
    $undefined->foo();
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
