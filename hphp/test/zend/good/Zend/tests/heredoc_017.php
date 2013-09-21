<?php 

$a = <<<A
	A;
;
 A;
\;
A;

var_dump(strlen($a) == 12);

?>