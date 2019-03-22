<?php
const pass3 = 1;
const pass2 = 1;
function errorHandler($errorNumber, $errorMessage, $fileName, $lineNumber) {
  include(__FILE__);
  die("Error: $errorMessage ($fileName:$lineNumber)\n");
}
if (defined("pass3")) {

  class ErrorClass {
  }

} else if (defined("pass2")) {

  class TestClass {
    function __construct() {
    }
    function TestClass() {
      $this->__construct();
    }
  }

} else {
  set_error_handler('errorHandler');
  include(__FILE__);
  print "ok\n";
}

