<?php

function my_error_handler($errno, $errstr, $errfile, $errline) {
	var_dump($errstr);
}

set_error_handler('my_error_handler');

class a
{
   public $a = 4;
   function __call($a,$b) {
       return "unknown method";
   }
}

$b = new a;
echo $b,"\n";
$c = unserialize(serialize($b));
echo $c,"\n";
var_dump($c);

?>