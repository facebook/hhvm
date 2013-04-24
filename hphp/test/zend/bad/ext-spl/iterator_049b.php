<?php

$ar = new ArrayIterator(array(
	NULL=>1, 
	"\0"=>2,
	"\0\0"=>3,
	"\0\0\0"=>4,
	"\0*"=>5,
	"\0*\0"=>6,
	));
@var_dump($ar);
var_dump($ar->getArrayCopy());

?>
===DONE===
<?php exit(0); ?>