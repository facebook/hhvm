<?php

echo "*** Testing addcslashes() for basic operations ***\n";
/* checking normal operation of addcslashes */
$string = "goodyear12345NULL\0truefalse\a\v\f\b\n\r\t";
$charlist = array ( 
  NULL,
  2,
  array(5,6,7),
  "a",
  "\0",
  "\n",
  "\r",
  "\t",
  "\a",
  "\v",
  "\b",
  "\f"
);
/* loop prints string with backslashes before characters
   mentioned in $char using addcslashes() */
$counter = 1;
foreach($charlist as $char) {
  echo "-- Iteration $counter --\n";
  try { var_dump( addcslashes($string, $char) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $counter++;
}

echo "Done\n"; 

?>
