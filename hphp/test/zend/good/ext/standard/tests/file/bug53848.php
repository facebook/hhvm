<?php
$file = dirname(__FILE__) . "/bug53848.csv";
@unlink($file);
file_put_contents($file, "a,b\n  c,  d");
$fp = fopen($file, "r");
while ($l = fgetcsv($fp)) var_dump($l);
fclose($fp);
@unlink($file);
?>