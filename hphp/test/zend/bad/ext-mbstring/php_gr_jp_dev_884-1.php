<?php
set_time_limit(2);
var_dump(ereg_replace(".*", "b", "a"));
var_dump(preg_replace("/.*/", "b", "a"));
var_dump(mb_ereg_replace(".*", "b", "a"));
?>