<?php

var_dump(ezc_realpath('.') == ezc_realpath(getcwd()));
chdir('..');
var_dump(ezc_realpath('.') == ezc_realpath(getcwd()));

?>
