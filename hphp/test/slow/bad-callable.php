<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Bad {}
function foo1() { return array_map(new Bad(), []); }
function foo2() { return array_filter([], new Bad()); }
function foo3() { return array_reduce([], new Bad()); }

<<__EntryPoint>>
function main_bad_callable() {
;
var_dump(foo1());
var_dump(foo2());
var_dump(foo3());
}
