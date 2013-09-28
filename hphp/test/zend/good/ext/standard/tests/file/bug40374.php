<?php

$file = tempnam(sys_get_temp_dir(), "test_");
var_dump($file);
$fp = fopen($file, "wt");
fwrite($fp, "test");
fclose($fp);
unlink($file);

echo "Done\n";
?>