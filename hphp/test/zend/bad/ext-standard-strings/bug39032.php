<?php

var_dump(strcspn(chr(0),"x"));
var_dump(strcspn(chr(0),""));
var_dump(strcspn(chr(0),"qweqwe"));
var_dump(strcspn(chr(1),"qweqwe"));

echo "Done\n";
?>