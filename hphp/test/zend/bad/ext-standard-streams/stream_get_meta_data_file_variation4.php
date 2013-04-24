<?php

echo "Create a file:\n";
$filename = __FILE__ . '.tmp';
$fp = fopen('File://' . $filename, 'w+');

var_dump(stream_get_meta_data($fp));

fclose($fp);

echo "\nChange to file's directory and open with a relative path:\n";

$dirname = dirname($filename);
chdir($dirname);
$relative_filename = basename($filename);

$fp = fopen($relative_filename, 'r');
var_dump(stream_get_meta_data($fp));

fclose($fp);

unlink($filename);

?>