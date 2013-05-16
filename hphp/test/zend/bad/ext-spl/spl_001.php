<?php

$it = new ArrayObject(array("x"=>1, 1=>2, 3=>3, 4, "1"=>5));

$ar = iterator_to_array($it);

var_dump(iterator_count($it));

print_r($ar);

foreach($ar as $v)
{
	var_dump($v);
}

?>
===DONE===