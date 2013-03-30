<?php

var_dump(trigger_error());
var_dump(trigger_error("error"));
var_dump(trigger_error(array()));
var_dump(trigger_error("error", -1));
var_dump(trigger_error("error", 0));
var_dump(trigger_error("error", E_USER_WARNING));
var_dump(trigger_error("error", E_USER_DEPRECATED));

echo "Done\n";
?>