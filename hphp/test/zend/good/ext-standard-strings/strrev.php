<?php
	$i = 0;
	$str = '';

	while ($i<256) {
		$str .= chr($i++);
	}
	
	var_dump(md5(strrev($str)));
	var_dump(strrev(NULL));
	var_dump(strrev(""));
?>