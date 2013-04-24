<?php

$filepath = __FILE__ . ".tmp";
$fd = fopen($filepath, "w+");
fwrite($fd, "Line 1\n\n \n  \n\Line 3");
fclose($fd);

echo "file():\n";
var_dump(file($filepath));

echo "\nfile() with FILE_IGNORE_NEW_LINES:\n";
var_dump(file($filepath, FILE_IGNORE_NEW_LINES));

echo "\nfile() with FILE_SKIP_EMPTY_LINES:\n";
var_dump(file($filepath, FILE_SKIP_EMPTY_LINES));

echo "\nfile() with FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES:\n";
var_dump(file($filepath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES));

unlink($filepath);

?>