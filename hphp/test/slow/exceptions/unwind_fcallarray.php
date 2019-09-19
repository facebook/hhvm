<?hh

function handle_error() {
  throw new Exception("die rebel scum");
}

function main() {
  set_error_handler(fun('handle_error'));
  date_default_timezone_set(...array('America/Gotham_City'));
}

main();
