<?php

$a = array(array());
$a[] =& $a;

var_dump(each($a[1]));

?>