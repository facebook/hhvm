<?php

var_dump(gettype(ini_get_all()));
var_dump(ini_get_all(""));
var_dump(ini_get_all("nosuchextension"));
var_dump(ini_get_all("reflection"));
var_dump(ini_get_all("pcre"));
var_dump(ini_get_all("pcre", false));
var_dump(ini_get_all("reflection", false));

var_dump(ini_get_all("", ""));

echo "Done\n";
?>