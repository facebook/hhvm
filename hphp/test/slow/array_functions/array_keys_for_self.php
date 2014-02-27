<?php
$a = array();
$a['self'] = &$a;
var_dump(array_keys($a, $a));
var_dump(array_keys($a, $a, /*strict =*/ TRUE));
