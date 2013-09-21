<?php

$file = dirname(__FILE__)."/001.test";
@unlink($file);

var_dump(imagecreatefrompng($file));
touch($file);
var_dump(imagecreatefrompng($file));

@unlink($file);

echo "Done\n";
?>