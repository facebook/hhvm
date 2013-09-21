<?php
/* Prototype  : int gmmktime([int hour [, int min [, int sec [, int mon [, int day [, int year]]]]]])
 * Description: Get UNIX timestamp for a GMT date 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing gmmktime() : usage variation ***\n";

//Initialise variables 
$hour = 8;
$min = 8;
$sec = 8;
$mon = 8;
$day = 8;
$year = 2008;

$inputs = array(

	  'float 123456' => 123456,
      'float -123456' => -123456,
      'float -10.5' => -10.5,
);

// loop through each element of the array for min
foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
	  var_dump( gmmktime($value, $min, $sec, $mon, $day, $year) );
	  var_dump( gmmktime($hour, $value, $sec, $mon, $day, $year) );
	  var_dump( gmmktime($hour, $min, $value, $mon, $day, $year) );
	  var_dump( gmmktime($hour, $min, $sec, $value, $day, $year) );
	  var_dump( gmmktime($hour, $min, $sec, $mon, $value, $value) );
}	  
?>
===DONE===