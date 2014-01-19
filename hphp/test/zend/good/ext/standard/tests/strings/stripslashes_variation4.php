<?php
/* Prototype  : string stripslashes ( string $str )
 * Description: Returns an un-quoted string
 * Source code: ext/standard/string.c
*/

/*
 * Test stripslashes() with double dimensional arrays
*/

echo "*** Testing stripslashes() : with double dimensional arrays ***\n";

// initialising the string array

$str_array = array( 
                    array("", array()),
                    array("", array("")),
                    array("f\\'oo", "b\\'ar", array("fo\\'o", "b\\'ar")),
                    array("f\\'oo", "b\\'ar", array("")),
                    array("f\\'oo", "b\\'ar", array("fo\\'o", "b\\'ar", array(""))),
                    array("f\\'oo", "b\\'ar", array("fo\\'o", "b\\'ar", array("fo\\'o", "b\\'ar")))
                  );
function stripslashes_deep($value)  {
  $value = is_array($value) ? array_map('stripslashes_deep', $value) : stripslashes($value);
  return $value;
}

$count = 1;
// looping to test for all strings in $str_array
foreach( $str_array as $arr )  {
  echo "\n-- Iteration $count --\n";
  var_dump( stripslashes_deep($arr) );
  $count ++;
}

echo "Done\n";
?>