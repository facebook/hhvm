<?php

var_dump(stream_filter_register("", ""));
var_dump(stream_filter_register("test", ""));
var_dump(stream_filter_register("", "test"));
var_dump(stream_filter_register("------", "nonexistentclass"));
var_dump(stream_filter_register(array(), "aa"));
var_dump(stream_filter_register("", array()));

echo "Done\n";
?>