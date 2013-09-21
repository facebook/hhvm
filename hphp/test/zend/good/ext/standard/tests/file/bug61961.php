<?php
$tmp_empty_file = __FILE__ . ".tmp";
file_put_contents($tmp_empty_file, "");

var_dump(file_get_contents($tmp_empty_file, NULL, NULL, NULL, 10));
unlink($tmp_empty_file);
?>
==DONE==