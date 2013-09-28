<?php
$a = array();
$b =& $a;
$a[0] = $a;
debug_zval_dump($a);
$a = array(array());
$b =& $a;
$a[0][0] = $a;
debug_zval_dump($a);
?>