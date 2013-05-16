<?php
$array = new SplFixedArray(5);
$a = $array->offsetUnset();
if(is_null($a)) {
	echo 'PASS';
}
?>