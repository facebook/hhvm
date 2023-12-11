<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Bad {}
function foo1() :mixed{ return array_map(new Bad(), vec[]); }
function foo2() :mixed{ return array_filter(vec[], new Bad()); }
function foo3() :mixed{ return array_reduce(vec[], new Bad()); }

<<__EntryPoint>>
function main_bad_callable() :mixed{
;
var_dump(foo1());
var_dump(foo2());
var_dump(foo3());
}
