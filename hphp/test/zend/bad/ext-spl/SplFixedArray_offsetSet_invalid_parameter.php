<?php
$array = new SplFixedArray(5);
$a = $array->offsetSet();
if(is_null($a)) {
	echo 'PASS';
}
?>