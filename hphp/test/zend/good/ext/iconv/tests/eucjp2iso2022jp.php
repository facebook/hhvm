<?php
/* include('test.inc'); */
/* charset=EUC-JP */

function hexdump($str) {
	$len = strlen($str);
	for ($i = 0; $i < $len; ++$i) {
		printf("%02x", ord($str{$i}));
	}
	print "\n";
}

$str = str_repeat("日本語テキストと English text", 30);
$str .= "日本語";

echo hexdump(iconv("EUC-JP", "ISO-2022-JP", $str));
?>