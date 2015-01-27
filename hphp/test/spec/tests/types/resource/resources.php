<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

var_dump(STDIN);
var_dump(is_resource(STDIN));
var_dump(get_resource_type(STDIN));

var_dump(STDOUT);
var_dump(is_resource(STDIN));
var_dump(get_resource_type(STDIN));

var_dump(STDERR);
var_dump(is_resource(STDIN));
var_dump(get_resource_type(STDIN));
