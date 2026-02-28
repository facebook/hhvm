<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

var_dump(HH\stdin());
var_dump(is_resource(HH\stdin()));
var_dump(get_resource_type(HH\stdin()));

var_dump(HH\stdout());
var_dump(is_resource(HH\stdout()));
var_dump(get_resource_type(HH\stdout()));

var_dump(HH\stderr());
var_dump(is_resource(HH\stderr()));
var_dump(get_resource_type(HH\stderr()));
}
