<?php
function test($url, $mode) {
	echo "$url, $mode\n";
	$fd = fopen($url, $mode);
	var_dump($fd, fwrite($fd, b"foo"));
	var_dump(fseek($fd, 0, SEEK_SET), fread($fd, 3));
	fclose($fd);
}
test("php://memory","r");
test("php://memory","r+");
test("php://temp","r");
test("php://temp","w");
?>