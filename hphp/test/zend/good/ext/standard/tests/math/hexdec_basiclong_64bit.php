<?php
 
define("MAX_64Bit", 9223372036854775807);
define("MAX_32Bit", 2147483647);
define("MIN_64Bit", -9223372036854775807 - 1);
define("MIN_32Bit", -2147483647 - 1);

$hexLongStrs = array(
   '7'.str_repeat('f',15), 
   str_repeat('f',16), 
   '7'.str_repeat('f',7), 
   str_repeat('f',8), 
   '7'.str_repeat('f',16),
   str_repeat('f',18),
   '7'.str_repeat('f',8), 
   str_repeat('f',9)
);


foreach ($hexLongStrs as $strVal) {
   echo "--- testing: $strVal ---\n";
   var_dump(hexdec($strVal));
}
   
?>
===DONE===