<?php
/* Prototype  : bool checkdate  ( int $month  , int $day  , int $year  )
 * Description: Checks the validity of the date formed by the arguments. 
 *              A date is considered valid if each parameter is properly defined. 
 * Source code: ext/date/php_date.c
 */

echo "*** Testing checkdate() : usage variation -  unexpected values to second argument \$day***\n";

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

// resource
$file_handle = fopen(__FILE__, 'r');

//array of values to iterate over
$inputs = array(
      // float data
      'float 10.5' => 10.5,
      'float -10.5' => -10.5,
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
      
      // resource 
      'resource' => $file_handle
);

$month = 7;
$year = 1963;

foreach($inputs as $variation =>$day) {
      echo "\n-- $variation --\n";
      try { var_dump( checkdate($month, $day, $year) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

// closing the resource
fclose( $file_handle);

?>
===DONE===
