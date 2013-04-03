<?php
function a($s){return $s;}
function b($s){return $s;}
function c($s){return $s;}
function d($s){return $s;}

ob_start();
ob_start('a');
ob_start('b');
ob_start('c');
ob_start('d');
ob_start();

echo "foo\n";

ob_flush();
ob_end_clean();
ob_flush();

print_r(ob_list_handlers());
print_r(ob_get_status());
print_r(ob_get_status(true));

?>