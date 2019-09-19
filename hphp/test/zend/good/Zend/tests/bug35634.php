<?hh
const pass3 = 1;
const pass2 = 1;
function errorHandler($errorNumber, $errorMessage, $fileName, $lineNumber) {
  include(__FILE__);
  die("Error: $errorMessage ($fileName:$lineNumber)\n");
}

<<__EntryPoint>>
function main_entry(): void {
  if (defined("pass3")) {
    include 'bug35634-1.inc';
  } else if (defined("pass2")) {
    include 'bug35634-2.inc';
  } else {
    set_error_handler(fun('errorHandler'));
    include(__FILE__);
    print "ok\n";
  }
}
