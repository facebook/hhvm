<?hh

// Stuff that doesn't typecheck

$saved_argv0 = $GLOBALS['argv'][0];
function error_UNSAFE($message) {
  $saved_argv0 = $GLOBALS['saved_argv0'];
  echo("$saved_argv0: $message\n");
  exit(1);
}

function parse_options_UNSAFE($optmap) {
  return parse_options_impl($optmap, &$GLOBALS['argv']);
}
