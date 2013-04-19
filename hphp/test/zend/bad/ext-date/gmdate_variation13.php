<?php
/* Prototype  : string gmdate(string format [, long timestamp])
 * Description: Format a GMT date/time 
 * Source code: ext/date/php_date.c
 */

echo "*** Testing gmdate() : usage variation ***\n";

// Initialise all required variables
date_default_timezone_set('UTC');
$timestamp = mktime(8, 8, 8, 8, 8, 2008);

//array of values to iterate over
$inputs = array(
      // Predefined Date constants
      'DATE_ATOM Constant' => DATE_ATOM,
	  'DATE_COOKIE Constant' => DATE_COOKIE,
	  'DATE_RFC822 Constant' => DATE_RFC822,
	  'DATE_RFC850 Constant' => DATE_RFC850,
	  'DATE_RFC1036 Constant' => DATE_RFC1036,
	  'DATE_RFC1123 Constant' => DATE_RFC1123,
	  'DATE_RFC2822 Constant' => DATE_RFC2822,
	  'DATE_RFC3339 Constant' => DATE_RFC3339,
	  'DATE_RSS Constant' => DATE_RSS,
	  'DATE_W3C Constant' => DATE_W3C,
);

// loop through each element of the array for format
foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
	  var_dump( gmdate($value, $timestamp) );
	  var_dump( gmdate($value) );
};

?>
===DONE===