<?php
/* Prototype  : string uniqid  ([ string $prefix= ""  [, bool $more_entropy= false  ]] )
 * Description: Gets a prefixed unique identifier based on the current time in microseconds. 
 * Source code: ext/standard/uniqid.c
*/
echo "*** Testing uniqid() : basic functionality ***\n";

echo "\nuniqid() without a prefix\n";
var_dump(uniqid());
var_dump(uniqid(null, true));
var_dump(uniqid(null, false));
echo "\n\n";

echo "uniqid() with a prefix\n";

// Use a fixed prefix so we can ensure length of o/p id is fixed 
$prefix = array (
				99999,
				"99999",
				10.5e2,
				null,
				true,
				false				
				);

for ($i = 0; $i < count($prefix); $i++) {				
	var_dump(uniqid($prefix[$i]));
	var_dump(uniqid($prefix[$i], true));
	var_dump(uniqid($prefix[$i], false));
	echo "\n";
}	

?>
===DONE===