<?php

$avail = UConverter::getAvailable();
var_dump(count($avail) > 100);
var_dump(in_array('UTF-7', $avail));
var_dump(in_array('CESU-8', $avail));
var_dump(in_array('ISO-8859-1', $avail));
$latin1 = UConverter::getAliases('latin1');
var_dump(in_array('ISO-8859-1', $latin1));
