<?hh

function handle_error() {
  throw new Exception("die rebel scum");
}
<<__EntryPoint>>
function main() {
  set_error_handler(fun('handle_error'));
  date_default_timezone_set(...varray['America/Gotham_City']);
}
