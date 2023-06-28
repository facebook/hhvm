<?hh
function myErrorHandler($errno, $errstr, $errfile, $errline) :mixed{
  echo "Error: $errstr at $errfile($errline)\n"; return true;
}
<<__EntryPoint>> function main(): void {
set_error_handler(myErrorHandler<>, E_RECOVERABLE_ERROR);
$x = function() { return 1; };
print (string) $x;
print "\n";
print $x;
print "\n";
}
