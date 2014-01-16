<?php
parse_str("dummy=42", $_POST);
$_REQUEST = array_merge($_REQUEST, $_POST);

if (false !== ob_gzhandler("", PHP_OUTPUT_HANDLER_START)) {
	ini_set("zlib.output_compression", 0);
	ob_start("ob_gzhandler");
}
echo "hi\n";
?>