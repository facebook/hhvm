<?php

$badvals = array(4294901761, 2147483648, -2147483648, -1);

foreach ($badvals as $val) {
	$x = numfmt_create("en", NumberFormatter::PATTERN_DECIMAL);
	var_dump(numfmt_set_symbol($x, $val, ""));
	var_dump(intl_get_error_message());
}

?>