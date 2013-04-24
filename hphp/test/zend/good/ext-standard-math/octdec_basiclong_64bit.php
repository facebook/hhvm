<?php
 
define("MAX_64Bit", 9223372036854775807);
define("MAX_32Bit", 2147483647);
define("MIN_64Bit", -9223372036854775807 - 1);
define("MIN_32Bit", -2147483647 - 1);

$octLongStrs = array(
   '777777777777777777777', 
   '1777777777777777777777', 
   '17777777777',
   '37777777777',
   '377777777777777777777777',
   '17777777777777777777777777',
   '377777777777',
   '777777777777',
);


foreach ($octLongStrs as $strVal) {
   echo "--- testing: $strVal ---\n";
   var_dump(octdec($strVal));
}
   
?>
===DONE===