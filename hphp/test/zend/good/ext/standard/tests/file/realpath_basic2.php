<?php

var_dump(realpath('.') == realpath(getcwd()));
chdir('..');
var_dump(realpath('.') == realpath(getcwd()));

?>