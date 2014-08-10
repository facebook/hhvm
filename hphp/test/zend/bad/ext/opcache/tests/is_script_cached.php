<?php
var_dump(opcache_is_script_cached(__FILE__));
var_dump(opcache_is_script_cached("nonexistent.php"));
?>