<?php
set_error_handler('myErrorHandler', E_RECOVERABLE_ERROR);
function myErrorHandler($errno, $errstr, $errfile, $errline) {
  echo "Error: $errstr at $errfile($errline)\n";
  return true;
}
$x = function() { return 1; };
print (string) $x;
print "\n";
print $x;
print "\n";
?>
