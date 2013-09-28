<?php
/* Prototype  : proto int fseek(resource fp, int offset [, int whence])
 * Description: Seek on a file pointer 
 * Source code: ext/standard/file.c
 * Alias to functions: gzseek
 */

echo "*** Testing fseek() : usage variations ***\n";
error_reporting(E_ALL & ~E_NOTICE);
$fp = fopen(__FILE__, 'r');
$offset = 3;

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//array of values to iterate over
$values = array(

      // outside of whence range
      -100,
      100,

      // float data
      10.5,
      -10.5,
      10.1234567e10,
      10.7654321E-10,
      .5,

      // null data
      NULL,
      null,

      // boolean data
      true,
      false,
      TRUE,
      FALSE,

      // empty data
      "",
      '',

      // string data
      "string",
      'string',

      // undefined data
      $undefined_var,

      // unset data
      $unset_var,
);

// loop through each element of the array for whence

foreach($values as $value) {
      echo "\nArg value $value \n";
      var_dump( fseek($fp, $offset, $value) );
      var_dump( ftell($fp));
};

fclose($fp);
echo "Done";
?>