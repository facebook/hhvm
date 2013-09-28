<?php

$a = array();
$a[] =& $a;
print_r($a);
apc_store('table', $a);
