<?php

var_dump(convert_uuencode(array()));
var_dump(convert_uudecode(array()));

var_dump(convert_uuencode(""));
var_dump(convert_uudecode(""));
var_dump($enc = convert_uuencode("~!@#$%^&*()_}{POIUYTREWQQSDFGHJKL:<MNBVCXZ"));
var_dump(convert_uudecode("!@#$%^YUGFDFGHJKLUYTFBNMLOYT"));
var_dump(convert_uudecode($enc));
var_dump($enc = convert_uuencode("not very sophisticated"));
var_dump(convert_uudecode($enc));
var_dump(convert_uudecode(substr($enc, 0, -10)));

echo "Done\n";
?>