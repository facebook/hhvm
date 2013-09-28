<?php

$filepath = __FILE__ . ".tmp";
$fd = fopen($filepath, "w+");
fwrite($fd, "Line 1\nLine 2\nLine 3");
fclose($fd);

for ($flags = 0; $flags <= 32; $flags++) {
	var_dump(file($filepath, $flags));
}

unlink($filepath);

?>