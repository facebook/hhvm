<?php
/* Prototype  : proto int fseek(resource fp, int offset [, int whence])
 * Description: Seek on a file pointer 
 * Source code: ext/standard/file.c
 * Alias to functions: gzseek
 */

echo "*** Testing fseek() : usage variations ***\n";
error_reporting(E_ALL & ~E_NOTICE);

// Initialise function arguments not being substituted (if any)

$fp = fopen(__FILE__, 'r');
$whence = SEEK_SET;

//get an unset variable
$unset_var = 10;
unset ($unset_var);

class testClass {
   public function __toString() {
      return "testClass";
   }
}


//array of values to iterate over
$values = array(

      // float data
      10.5,
      -10.5,
      10.7654321E-10,
      .5,

      // array data
      array(),
      array(0),
      array(1),
      array(1, 2),
      array('color' => 'red', 'item' => 'pen'),

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
      
      // object data
      new testClass(),      

      // undefined data
      $undefined_var,

      // unset data
      $unset_var,
);

// loop through each element of the array for offset

foreach($values as $value) {
      echo @"\nArg value $value \n";
      var_dump( fseek($fp, $value, $whence) );
      var_dump( ftell($fp));
};
fclose($fp);

echo "Done";
?>
