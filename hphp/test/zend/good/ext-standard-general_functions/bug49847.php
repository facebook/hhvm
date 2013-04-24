<?php
$iswin =  substr(PHP_OS, 0, 3) == "WIN";

if ($iswin) {
	$f = dirname(__FILE__) . '\\bug49847.tmp';
	$s = str_repeat(' ', 4097);
	$s .= '1';
	file_put_contents($f, $s);
	exec('type ' . $f, $output);
} else {
	exec("printf %4098d 1", $output);
}
var_dump($output);
if ($iswin) {
	unlink($f);
}
?>