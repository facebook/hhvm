<?php

var_dump(strncmp("", ""));
var_dump(strncmp("", "", 100));
var_dump(strncmp("aef", "dfsgbdf", -1));
var_dump(strncmp("fghjkl", "qwer", 0));
var_dump(strncmp("qwerty", "qwerty123", 6));
var_dump(strncmp("qwerty", "qwerty123", 7));

echo "Done\n";
?>