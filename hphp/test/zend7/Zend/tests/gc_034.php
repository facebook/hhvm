<?php
/* run with valgrind */
$a = array(fopen(__FILE__, 'r'));
$a[] = &$a;
?>
==DONE==
