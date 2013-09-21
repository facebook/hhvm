<?php

$filename = dirname(__FILE__)."/fwrite.dat";

$fp = fopen($filename, "w");
var_dump(fwrite($fp));
var_dump(fwrite($fp, array()));
fclose($fp);

$fp = fopen($filename, "r");
var_dump(fwrite($fp, "data"));

$fp = fopen($filename, "w");
var_dump(fwrite($fp, "data", -1));
var_dump(fwrite($fp, "data", 100000));
fclose($fp);

var_dump(fwrite($fp, "data", -1));

var_dump(fwrite(array(), "data", -1));
var_dump(fwrite(array(), "data"));
var_dump(fwrite(array()));

var_dump(file_get_contents($filename));

@unlink($filename);
echo "Done\n";
?>