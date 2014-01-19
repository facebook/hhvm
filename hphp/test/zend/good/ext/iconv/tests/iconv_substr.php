<?php
function hexdump($str) {
	$len = strlen($str);
	for ($i = 0; $i < $len; ++$i) {
		printf("%02x", ord($str[$i]));
	}
	print "\n";
}

function foo($str, $offset, $len, $charset) {
	hexdump(substr($str, $offset, $len));
	hexdump(iconv_substr($str, $offset, $len, $charset));
}

function bar($str, $offset, $len = false) {
	if (is_bool($len)) {
		var_dump(substr($str, $offset));
		var_dump(iconv_substr($str, $offset));
	} else {
		var_dump(substr($str, $offset, $len));
		var_dump(iconv_substr($str, $offset, $len));
	}
}

foo("abcdefghijklmnopqrstuvwxyz", 5, 7, "ASCII");
foo("あいうえおかきくけこさしす", 5, 7, "EUC-JP");
bar("This is a test", 100000);
bar("This is a test", 0, 100000);
bar("This is a test", -3);
bar("This is a test", 0, -9);
bar("This is a test", 0, -100000);
bar("This is a test", -9, -100000);
var_dump(iconv("ISO-2022-JP", "EUC-JP", iconv_substr(iconv("EUC-JP", "ISO-2022-JP", "こんにちは ISO-2022-JP"), 3, 8, "ISO-2022-JP")));
?>