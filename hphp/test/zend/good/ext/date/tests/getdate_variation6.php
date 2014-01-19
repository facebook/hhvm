<?php
/* Prototype  : array getdate([int timestamp])
 * Description: Get date/time information 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing getdate() : usage variation ***\n";

date_default_timezone_set("Asia/Calcutta");

//Timezones with required data for date_sunrise
$inputs = array (
		'String 0' => '0',
		'String 10.5' => "10.5",
		'String -10.5' => '-10.5',
);

// loop through each element of the array for timestamp
foreach($inputs as $key => $value) {
      echo "\n--$key--\n";
      var_dump( getdate($value) );
};
?>
===DONE===