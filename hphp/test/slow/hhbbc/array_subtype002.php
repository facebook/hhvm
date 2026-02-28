<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo1() :mixed{ return 0; }
function foo2() :mixed{ return 1; }
function foo3() :mixed{ return 2; }
function blah($x) :mixed{ return dict[foo1() => $x, foo2() => $x, foo3() => $x]; }
function main() :mixed{ var_dump(blah('abc')); }

<<__EntryPoint>>
function main_array_subtype002() :mixed{
main();
}
