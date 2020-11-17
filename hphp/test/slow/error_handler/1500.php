<?hh
function handler ($errno, $errstr, $errfile, $errline, darray $errcontext) {
  echo "handler_called\n";
}
<<__EntryPoint>> function main(): void {
set_error_handler(handler<>);
$undefined->foo();
}
