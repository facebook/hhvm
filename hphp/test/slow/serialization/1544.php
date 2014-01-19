<?php

$a = array(array());
$a[0][0] = &$a[0];
var_dump(serialize($a));
