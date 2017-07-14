<?php

 //php.ini: error_reporting = E_ALL | E_DEPRECATED | E_STRICT

 class ErrorHandling {

  public  function error_handler($errno, $errstr, $errfile, $errline) {
	  $bla = new NonExistingClass2();
  }

  public function exception_handler(Error $e) { 
	  echo "Caught, exception: " . $e->getMessage();
  }
 }

 set_error_handler('ErrorHandling::error_handler');
 set_exception_handler('ErrorHandling::exception_handler');

 $blubb = new NonExistingClass();
?>
