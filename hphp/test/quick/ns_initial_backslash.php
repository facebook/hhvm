<?php
error_reporting(-1);

define('\B', 1);
var_dump(B);

const B = 1;
var_dump(B);

var_dump(constant('B'));
var_dump(constant('\B'));

var_dump(defined('B'));
var_dump(defined('\B'));
