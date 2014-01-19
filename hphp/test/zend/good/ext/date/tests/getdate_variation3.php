<?php
/* Prototype  : array getdate([int timestamp])
 * Description: Get date/time information 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing getdate() : usage variation ***\n";

//Set the default time zone 
date_default_timezone_set("Asia/Calcutta");

//array of values to iterate over
$inputs = array(

	//octal values
	'hexadcimal 0x5' => 0x5,
	'hexadcimal 0xCAFE' => 0xCAFE,
	'octal -0xCAFE' => -0xCAFE,
);

// loop through each element of the array for timestamp

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( getdate($value) );
};

?>
===DONE===