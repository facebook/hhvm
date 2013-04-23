<?php

$string = urldecode("search%e4"); 
$result = preg_replace("#(&\#x*)([0-9A-F]+);*#iu","$1$2;",$string); 
var_dump($result); 
var_dump(preg_last_error());

echo "Done\n";
?>