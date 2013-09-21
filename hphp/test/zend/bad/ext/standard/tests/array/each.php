<?php
/* Return the current key and value pair from an array 
   and advance the array cursor */

echo "*** Testing each() : basic functionality ***\n";
$arrays = array (
	    array(0),
	    array(1),
	    array(-1),
	    array(1, 2, 3, 4, 5),
	    array(-1, -2, -3, 6, 7, 8),
 	    array("a", "ab", "abc", "abcd"),
	    array("1" => "one", "2" => "two", "3" => "three", "4" => "four"),
	    array("one" => 1, "two" => 2, 3 => "three", 4 => 4, "" => 5, 
		  "  " => 6, "\x00" => "\x000", "\0" => "\0", "" => "",
		  TRUE => TRUE, FALSE => FALSE, NULL => NULL
		 ),
	    array("1.5" => "one.5", "-2.0" => "negative2.0"),
	    array(-5 => "negative5", -.05  => "negative.05")
	  );

/* loop through to check working of each() on different arrays */
$i = 0;
while( list( $key, $sub_array) = each($arrays) )  {
  echo "-- Iteration $i --\n";
  $c = count ($sub_array);
  $c++; 		// increment by one to create the situation 
			// of accessing beyond array size
  while ( $c ) {
    var_dump( each($sub_array) );
    $c --;
  }
  /* assignment of an array to another variable resets the internal 
     pointer of the array. check this and ensure that each returns 
     the first element after the assignment */
  $new_array = $sub_array;
  var_dump( each($sub_array) );
  echo "\n";
  $i++;
}

echo "\n*** Testing each() : possible variations ***\n";
echo "-- Testing each() with reset() function --\n";
/* reset the $arrays and use each to get the first element */ 
var_dump( reset($arrays) );
var_dump( each($arrays) );  // first element
list($key, $sub_array) = each($arrays);  // now second element
var_dump( each($sub_array) );


echo "-- Testing each() with resources --\n";
$fp = fopen(__FILE__, "r");
$dfp = opendir(".");

$resources = array (
  "file" => $fp,
  "dir" => $dfp
);

for( $i = 0; $i < count($resources); $i++) {
  var_dump( each($resources) );
}

echo "-- Testing each with objects --\n";

class each_class {
  private $var_private = 100;
  protected $var_protected = "string";
  public $var_public = array(0, 1, TRUE, NULL);
}
$each_obj = new each_class();
for( $i = 0; $i <= 2; $i++ ) {
  var_dump( each($each_obj) );
}

echo "-- Testing each() with null array --\n";
$null_array = array();
var_dump( each($null_array) );


echo "\n*** Testing error conditions ***\n";

/* unexpected number of arguments */
var_dump( each() );  // args = 0
var_dump( each($null_array, $null_array) );  // args > expected

/* unexpected argument type */
$var=1;
$str ="string";
$fl = "15.5";
var_dump( each($var) );
var_dump( each($str) );
var_dump( each($fl) );

// close resourses used
fclose($fp);
closedir($dfp);

echo "Done\n";
?>
