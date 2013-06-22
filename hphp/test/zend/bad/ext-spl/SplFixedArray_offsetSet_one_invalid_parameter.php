<?php
$array = new SplFixedArray(5);
$a = $array->offsetSet(2);
if(is_null($a)) {
	echo 'PASS';
}
?>