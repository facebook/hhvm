<?hh

function handler(
  $errno,
  $errstr,
  $errfile,
  $errline,
  $deprecated_errctx,
  $errtrace,
  $ic_blame,
) {
  Errors::handle($errstr);
  handle_error($errno);
}

<<__EntryPoint>>
function main() {
  set_error_handler(handler<>, E_ALL);
  hello();
}
