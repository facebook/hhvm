<?php
function separate_zval(&$var) { }
<<__EntryPoint>> function main() {
$s = fopen(__FILE__, "rb"); $s2 = $s;
separate_zval(&$s2);
fclose($s);
echo fread($s2, strlen("<?php"));
echo "\nDone.\n";
}
