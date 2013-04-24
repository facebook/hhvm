<?php

$filename = __FILE__ . '.tmp';

$fp = fopen($filename, "w+");

echo "Write some data to the file:\n";
$i = 0;
while ($i++ < 20) {
	fwrite($fp, "a line of data\n");
}

var_dump(stream_get_meta_data($fp));

//seek to start of file
rewind($fp);

echo "\n\nRead a line of the file, causing data to be buffered:\n";
var_dump(fgets($fp));

var_dump(stream_get_meta_data($fp));

echo "\n\nRead 20 bytes from the file:\n";
fread($fp, 20);

var_dump(stream_get_meta_data($fp));

echo "\n\nRead entire file:\n";
while(!feof($fp)) {
	fread($fp, 1);
}

var_dump(stream_get_meta_data($fp));

fclose($fp);

unlink($filename);

?>