<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

var_dump(STDIN);
var_dump(is_resource(STDIN));
var_dump(get_resource_type(STDIN));

var_dump(STDOUT);
var_dump(is_resource(STDOUT));
var_dump(get_resource_type(STDOUT));

var_dump(STDERR);
var_dump(is_resource(STDERR));
var_dump(get_resource_type(STDERR));
}
