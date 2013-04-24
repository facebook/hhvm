<?php
$array = new SplFixedArray(5);
$a = $array->offsetExists();
if(is_null($a)) {
	echo 'PASS';
}
?>