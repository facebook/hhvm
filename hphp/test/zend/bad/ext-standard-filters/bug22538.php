<?php
function my_stream_copy_to_stream($fin, $fout) {
	while (!feof($fin)) {
		fwrite($fout, fread($fin, 4096));
	}
}

$size = 65536;

do {
	$path1 = sprintf("%s/%s%da", dirname(__FILE__), uniqid(), time());
	$path2 = sprintf("%s/%s%db", dirname(__FILE__), uniqid(), time());
} while ($path1 == $path2);

$fp = fopen($path1, "w") or die("Can not open $path1\n");
$str = "abcdefghijklmnopqrstuvwxyz\n";
$str_len = strlen($str);
$cnt = $size;
while (($cnt -= $str_len) > 0) {
	fwrite($fp, $str);
}
$cnt = $size - ($str_len + $cnt);
fclose($fp);
$fin = fopen($path1, "r") or die("Can not open $path1\n");;
$fout = fopen($path2, "w") or die("Can not open $path2\n");;
stream_filter_append($fout, "string.rot13");
my_stream_copy_to_stream($fin, $fout);
fclose($fout);
fclose($fin);
var_dump($cnt);
var_dump(filesize($path2));
var_dump(md5_file($path1));
var_dump(md5_file($path2));
unlink($path1);
unlink($path2);
?>