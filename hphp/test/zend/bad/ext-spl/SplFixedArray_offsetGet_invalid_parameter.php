<?php
$array = new SplFixedArray(5);
$array[0] = 'a';
$a = $array->offsetGet();
if(is_null($a)) {
	echo 'PASS';
}
?>