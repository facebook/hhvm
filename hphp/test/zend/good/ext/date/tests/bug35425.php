<?php

$time = mktime(1,1,1,1,1,2005);
foreach (array('B','d','h','H','i','I','L','m','s','t','U','w','W','y','Y','z','Z') as $v) {
	var_dump(idate($v, $time));
}

?>