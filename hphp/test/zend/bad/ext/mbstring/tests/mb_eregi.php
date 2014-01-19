<?php

mb_regex_encoding('utf-8');

var_dump(mb_eregi('z', 'XYZ'));
var_dump(mb_eregi('xyzp', 'XYZ'));
var_dump(mb_eregi('ö', 'Öäü'));
?>