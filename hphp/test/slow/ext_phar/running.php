<?php

var_dump(method_exists(Phar, running));
var_dump(Phar::running());
include __DIR__."/running.phar";

