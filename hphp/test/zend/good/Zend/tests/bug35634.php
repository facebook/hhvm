<?hh
const pass3 = 1;
const pass2 = 1;

function errorHandler($errorNumber, $errorMessage, $fileName, $lineNumber) :mixed{
  include(__FILE__);
  exit("Error: $errorMessage ($fileName:$lineNumber)\n");
}

<<__EntryPoint>>
function main_entry(): void {
  if (defined("pass3")) {
    include 'bug35634-1.inc';
  } else if (defined("pass2")) {
    include 'bug35634-2.inc';
  } else {
    set_error_handler(errorHandler<>);
    include(__FILE__);
    print "ok\n";
  }
}
