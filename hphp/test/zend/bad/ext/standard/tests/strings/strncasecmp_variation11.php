<?php
/* Prototype  : int strncasecmp ( string $str1, string $str2, int $len );
 * Description: Binary safe case-insensitive string comparison of the first n characters
 * Source code: Zend/zend_builtin_functions.c
*/

/* Test strncasecmp() function with the unexpected inputs for 'str2' */

echo "*** Testing strncasecmp() function: with unexpected values for 'str2' ***\n";
/* get an unset variable */
$unset_var = 'string_val';
unset($unset_var);

/* get resource handle */
$file_handle = fopen(__FILE__, "r");

/* declaring a class */
class sample  {
  public function __toString() {
  return "object";
  }
}


/* array with different values */
$values =  array (
  /* integer values */
  0,
  1,
  12345,
  -2345,
 
  /* float values */
  10.5,
  -10.5,
  10.5e10,
  10.6E-10,
  .5,

  /* hexadecimal values */
  0x12,
  -0x12,
  
  /* octal values */
  012,
  -012,
  01.2,

  /* array values */
  array(),
  array(0),
  array(1),
  array(1, 2),
  array('color' => 'red', 'item' => 'pen'),

  /* boolean values */
  true,
  false,
  TRUE,
  FALSE,

  /* nulls */
  NULL,
  null,

  /* empty string */
  "",
  '',

  /* undefined variable */
  @$undefined_var,

  /* unset variable */
  @$unset_var,

  /* resource */
  $file_handle,  

  /* object */
  new sample()
);

/* loop through each element of the array and check the working of strncasecmp() */
$counter = 1;
for($index = 0; $index < count($values); $index ++) {
  echo "-- Iteration $counter --\n";
  $str1 = $values[$index];
  $str2 = $values[$index];
  $len = strlen($values[$index]) + 1;
  var_dump( strncasecmp("string", $str2, $len) );
  $counter ++;
}

fclose($file_handle);  //closing the file handle

echo "*** Done ***\n";
?>