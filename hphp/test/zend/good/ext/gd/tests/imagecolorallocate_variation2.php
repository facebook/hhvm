<?php
/* Prototype  : int imagecolorallocate(resource im, int red, int green, int blue)
 * Description: Allocate a color for an image
 * Source code: ext/gd/gd.c
 */

echo "*** Testing imagecolorallocate() : usage variations ***\n";

$im = imagecreatetruecolor(200, 200);
$green = 10;
$blue = 10;

$fp = tmpfile();

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
$values = array(

      // float data
      'float 10.5' => 10.5,
      'float -10.5' => -10.5,
      'float 10.1234567e10' => 10.1234567e10,
      'float 10.7654321E-10' => 10.7654321E-10,
      'float .5' => .5,

      // array data
      'empty array' => array(),
      'int indexed array' => $index_array,
      'associative array' => $assoc_array,
	  'nested arrays' => array('foo', $index_array, $assoc_array),
      
      // null data
	  'uppercase NULL' => NULL,
      'lowercase null' => null,

      // boolean data
      'lowercase true' => true,
      'lowercase false' =>false,
      'uppercase TRUE' =>TRUE,
      'uppercase FALSE' =>FALSE,

      // empty data
      'empty string DQ' => "",
      'empty string SQ' => '',

      // string data
      'string DQ' => "string",
      'string SQ' => 'string',
      'mixed case string' => "sTrInG",
      'heredoc' => $heredoc,

      // object data
      'instance of classWithToString' => new classWithToString(),
      'instance of classWithoutToString' => new classWithoutToString(),

      // undefined data
      'undefined var' => @$undefined_var,

      // unset data
      'unset var' => @$unset_var,
      
      //resource 
      "file resource" => $fp
);
// loop through each element of the array for red
foreach($values as $key => $value) {
      echo "\n--$key--\n";
      var_dump( imagecolorallocate($im, $value, $green, $blue) );
};
?>
===DONE===
