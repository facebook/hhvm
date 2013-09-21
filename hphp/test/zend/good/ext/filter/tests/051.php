<?php
$tmp = $default = 321;
var_dump(filter_var("123asd", FILTER_VALIDATE_INT, array("options"=>array("default"=>$default))));
?>