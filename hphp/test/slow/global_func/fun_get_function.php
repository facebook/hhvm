<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function func() :mixed{}

<<__DynamicallyCallable>>
function dyn_func() :mixed{}
<<__EntryPoint>> function main(): void {
$dyn_func = 'dyn_func';
var_dump(HH\fun_get_function(func<>));
var_dump(HH\fun_get_function(HH\dynamic_fun($dyn_func)));
}
