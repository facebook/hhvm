<?hh

function handle_error() :mixed{
  throw new Exception("die rebel scum");
}
<<__EntryPoint>>
function main() :mixed{
  set_error_handler(handle_error<>);
  date_default_timezone_set(...vec['America/Gotham_City']);
}
