<?php


$filename = dirname(__FILE__)."/touch_variation2.dat";
$fp=fopen($filename,"w");
fwrite ($fp,"mydata");
fclose($fp);

var_dump(touch($filename, 101));
var_dump(file_get_contents($filename));

@unlink($filename);
echo "Done\n";

?>
