<?php
/* 
 * proto mixed microtime([bool get_as_float])
 * Function is implemented in ext/standard/microtime.c
*/ 

$opt_arg_0 = true;
$extra_arg = 1;

echo "\n-- Too many arguments --\n";
var_dump(microtime($opt_arg_0, $extra_arg));


echo "\n-- Bad Arg types --\n";

$bad_args = array(null,
				  1.5,
				  "hello",
				  array('k'=>'v', array(0)),
				  new stdClass,
				  1);
foreach ($bad_args as $bad_arg) {
	echo "\n--> bad arg: ";
	var_dump($bad_arg);
	var_dump(microtime($bad_arg));
}

?>
===DONE===