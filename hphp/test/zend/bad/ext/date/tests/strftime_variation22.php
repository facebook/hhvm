<?php
/* Prototype  : string strftime(string format [, int timestamp])
 * Description: Format a local time/date according to locale settings 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing strftime() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
setlocale(LC_ALL, "POSIX");
putenv("LC_ALL=POSIX");
date_default_timezone_set("Asia/Calcutta");
$timestamp = mktime(8, 8, 8, 8, 8, 2008);

//array of values to iterate over
$inputs = array(
      'Preferred date and time representation' => "%c",
	  'Preferred date representation' => "%x",
	  'Preferred time representation' => "%X",
);

// loop through each element of the array for timestamp

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
	  var_dump( $value );
	  var_dump( strftime($value, $timestamp) );
}	  

?>
===DONE===