<?php
var_dump(str_getcsv("0\t\t\"2\"\n", "\t"));
var_dump(str_getcsv("0\t \t'2'\n", "\t", "'"));
var_dump(str_getcsv(",,,,"));
var_dump(str_getcsv(" \t  \t\t\t ", "\t"));
?>