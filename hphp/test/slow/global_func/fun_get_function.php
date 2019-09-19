<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function func() {}

<<__DynamicallyCallable>>
function dyn_func() {};
<<__EntryPoint>> function main(): void {
$dyn_func = 'dyn_func';
var_dump(HH\fun_get_function(fun('func')));
var_dump(HH\fun_get_function(HH\dynamic_fun($dyn_func)));
}
