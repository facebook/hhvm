<?php
$s = fopen(__FILE__, "rb");
function separate_zval(&$var) { }
$s2 = $s;
separate_zval($s2);
fclose($s);
echo fread($s2, strlen("<?php"));
echo "\nDone.\n";
