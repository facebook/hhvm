<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo1() :mixed{ return "key"; }
function foo2() :mixed{ return "value"; }
function blah() :mixed{ return dict[foo1() => foo2()]; }
function main() :mixed{ var_dump(blah()); }

<<__EntryPoint>>
function main_array_subtype003() :mixed{
main();
}
