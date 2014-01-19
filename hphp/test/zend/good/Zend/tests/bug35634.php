<?php
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

  function errorHandler($errorNumber, $errorMessage, $fileName, $lineNumber) {
    define("pass3", 1);
    include(__FILE__);
    die("Error: $errorMessage ($fileName:$lineNumber)\n");
  }

  set_error_handler('errorHandler');
  define("pass2", 1);
  include(__FILE__);
  print "ok\n";
}

?>