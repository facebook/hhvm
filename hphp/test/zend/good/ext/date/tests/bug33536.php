<?php
date_default_timezone_set("GMT");
var_dump(strtotime("monkey"));
print date("Y-m-d", strtotime("monkey")) ."\n";
print date("Y-m-d", false) ."\n";
?>