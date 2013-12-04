<?php

$string = "goodyear12345NULL\0truefalse\a\v\f\b\n\r\t";
/* charlist "\0..\37" would escape all characters with ASCII code between 0 and 31 */
echo "\n*** Testing addcslashes() with ASCII code between 0 and 31 ***\n";
var_dump( addcslashes($string, "\0..\37") );

/* Checking OBJECTS type */
echo "\n*** Testing addcslashes() with objects ***\n";
class string1
{
  public function __toString() {
    return "Object";
  }
}
$obj = new string1;
var_dump( addcslashes($obj, "b") );

echo "Done\n"; 

?>