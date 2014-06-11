<?php

// Stuff that doesn't typecheck due to using 'global $foo';

$saved_argv0 = $GLOBALS['argv'][0];
function error_UNSAFE($message) {
  global $saved_argv0;
  echo("$saved_argv0: $message\n");
  exit(1);
}

function parse_options_UNSAFE($optmap) {
  global $argv;
  return parse_options_impl($optmap, $argv);
}
