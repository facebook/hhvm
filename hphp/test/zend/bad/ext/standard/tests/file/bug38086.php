<?php
define('WIN', substr(PHP_OS, 0, 3) == 'WIN');

$initial_file = dirname(__FILE__).'/bug38086.txt';
$new_file = dirname(__FILE__).'/bug38086_1.txt';

$src = fopen($initial_file, 'r');
stream_filter_append($src, "string.rot13", STREAM_FILTER_READ);

$dest = fopen($new_file, 'w');
var_dump(stream_copy_to_stream($src, $dest));
fclose($src); fclose($dest);

if (WIN) {
  var_dump(str_replace("\r\n","\n", file_get_contents($new_file)));
} else {
  var_dump(file_get_contents($new_file));
}
unlink($new_file);

$src = fopen($initial_file, 'r');
stream_filter_append($src, "string.rot13", STREAM_FILTER_READ);

$dest = fopen($new_file, 'w');
var_dump(stream_copy_to_stream($src, $dest, 10000));
fclose($src); fclose($dest);

if (WIN) {
  var_dump(str_replace("\r\n","\n", file_get_contents($new_file)));
} else {
  var_dump(file_get_contents($new_file));
}
unlink($new_file);

echo "Done\n";
?>