<?php

var_dump(strncmp("test ", "e", -1));
var_dump(strncmp("test ", "e", 10));
var_dump(strncmp("test ", "e", 0));

var_dump(strncasecmp("test ", "E", -1));
var_dump(strncasecmp("test ", "E", 10));
var_dump(strncasecmp("test ", "E", 0));

echo "Done\n";
?>