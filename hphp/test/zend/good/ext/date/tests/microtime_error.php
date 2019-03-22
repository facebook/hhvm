<?php
/* 
 * proto mixed microtime([bool get_as_float])
 * Function is implemented in ext/standard/microtime.c
*/ 

$opt_arg_0 = true;
$extra_arg = 1;

echo "\n-- Too many arguments --\n";
try { var_dump(microtime($opt_arg_0, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


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
	try { var_dump(microtime($bad_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

echo "===DONE===\n";
