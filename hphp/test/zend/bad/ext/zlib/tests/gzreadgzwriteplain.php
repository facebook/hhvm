<?php
$original = str_repeat(b"hallo php",4096);
$filename = tempnam("/tmp", "phpt");

$fp = fopen($filename, "wb");
fwrite($fp, $original);
var_dump(strlen($original));
var_dump(ftell($fp));
fclose($fp);

$fp = gzopen($filename, "rb");

$data = '';
while ($buf = gzread($fp, 8192)) {
	$data .= $buf;
}

if ($data == $original) {
	echo "Strings are equal\n";
} else {
	echo "Strings are not equal\n";
	var_dump($data);
}

gzseek($fp, strlen($original) / 2);

$data = '';
while ($buf = gzread($fp, 8192)) {
	$data .= $buf;
}

var_dump(strlen($data));
if ($data == substr($original, strlen($original) / 2)) {
	echo "Strings are equal\n";
} else {
	echo "Strings are not equal\n";
	var_dump($data);
}

gzclose($fp);
unlink($filename);
?>