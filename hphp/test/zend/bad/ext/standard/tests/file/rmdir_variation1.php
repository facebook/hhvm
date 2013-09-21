<?php
/* Prototype  : bool rmdir(string dirname[, resource context])
 * Description: Remove a directory 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing rmdir() : usage variation ***\n";

// Define error handler
function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) {
	if (error_reporting() != 0) {
		// report non-silenced errors
		echo "Error: $err_no - $err_msg, $filename($linenum)\n";
	}
}
set_error_handler('test_error_handler');


//get an unset variable
$unset_var = 10;
unset ($unset_var);

// define some classes
class classWithToString
{
	public function __toString() {
		return "Class A object";
	}
}

class classWithoutToString
{
}

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// add arrays
$index_array = array (1, 2, 3);
$assoc_array = array ('one' => 1, 'two' => 2);

//array of values to iterate over
$inputs = array(

      // null data
      'uppercase NULL' => NULL,
      'lowercase null' => null,

      // boolean data
      'lowercase false' =>false,
      'uppercase FALSE' =>FALSE,

      // empty data
      'empty string DQ' => "",
      'empty string SQ' => '',

      // undefined data
      'undefined var' => @$undefined_var,

      // unset data
      'unset var' => @$unset_var,
      
      // other
      'single space' => ' ',
);

// loop through each element of the array for dirname

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      var_dump(rmdir($value));
};

?>
===DONE===