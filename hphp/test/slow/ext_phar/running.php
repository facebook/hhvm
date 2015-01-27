<?php

var_dump(method_exists("Phar", "running"));
var_dump(Phar::running());
var_dump(Phar::running(false));
var_dump(Phar::running(true));
include __DIR__."/running.phar";
