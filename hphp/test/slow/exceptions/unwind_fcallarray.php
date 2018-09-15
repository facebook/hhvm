<?hh

function handle_error() {
  throw new Exception("die rebel scum");
}

function main() {
  set_error_handler('handle_error');
  apc_store('foo', 'bar');
  $success = null;
  $x = apc_fetch(...array('quux', $success));
}

main();
