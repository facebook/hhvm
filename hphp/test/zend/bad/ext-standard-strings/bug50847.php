<?php
$var = '<param value="' . str_repeat("a", 2048) . '" />';
var_dump(strip_tags($var, "<param>"), strip_tags($var));
?>