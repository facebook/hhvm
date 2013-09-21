<?php

var_dump(filter_id("stripped"));
var_dump(filter_id("string"));
var_dump(filter_id("url"));
var_dump(filter_id("int"));
var_dump(filter_id("none"));
var_dump(filter_id(array()));
var_dump(filter_id(-1));
var_dump(filter_id(0,0,0));

echo "Done\n";
?>