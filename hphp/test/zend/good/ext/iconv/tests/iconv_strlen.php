<?php
function foo($str, $charset) {
	var_dump(strlen($str));
	var_dump(iconv_strlen($str, $charset));
}

foo("abc", "ASCII");
foo("���ܸ� EUC-JP", "EUC-JP");
?>