<?php
$content = array("\"\nthis is an test\", \"next data\", \"p\narsed\"","\"\r\nthis is an test\", \"next data\", \"p\r\narsed\"","\"\n\rthis is an test\", \"next data\", \"p\n\rarsed\"");

$file = dirname(__FILE__) . "/bug39538.csv";
@unlink($file);
foreach ($content as $v) {
	file_put_contents($file, $v);
	print_r (fgetcsv(fopen($file, "r"), filesize($file)));
}
@unlink($file);
?>