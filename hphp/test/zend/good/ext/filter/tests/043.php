<?php 
$flags = FILTER_FLAG_ENCODE_AMP|FILTER_FLAG_ENCODE_LOW|FILTER_FLAG_ENCODE_HIGH;

for ($i = 0; $i < 256; $i++) {
	var_dump(filter_var(chr($i), FILTER_SANITIZE_STRING, array("flags" => $flags)));
}
?>