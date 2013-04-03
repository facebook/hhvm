<?php
error_reporting(E_ALL);
set_error_handler('kalus_error_handler');
ini_set("display_errors", "on");

class A {
  function A_ftk($a) {
  }
}

function kalus_error_handler($error_code, $error_string, $filename, $line, $symbols) {
  echo "$error_string\n";
  get_error_context();
}

function get_error_context() {
  $backtrace = debug_backtrace();
  $n = 1;
  foreach ($backtrace as $call) {
  	echo $n++." ";
  	if (isset($call["file"])) {
  		echo $call["file"];
  		if (isset($call["line"])) {
  			echo ":".$call["line"];
  		}
  	}
  	if (isset($call["function"])) {
  		echo " ".$call["function"]."()";
  	}
  	echo "\n";
  }
  echo "\n";
}

//This will not create file and line items for the call into the error handler
$page["name"] = A::A_ftk();
?>