<?php

var_dump(strcasecmp(""));
var_dump(strcasecmp("", ""));
var_dump(strcasecmp("aef", "dfsgbdf"));
var_dump(strcasecmp("qwe", "qwer"));
var_dump(strcasecmp("qwerty", "QweRty"));
var_dump(strcasecmp("qwErtY", "qwerty"));
var_dump(strcasecmp("q123", "Q123"));
var_dump(strcasecmp("01", "01"));

echo "Done\n";
?>