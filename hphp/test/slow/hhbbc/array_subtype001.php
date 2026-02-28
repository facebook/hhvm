<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo() :mixed{ return 123; }
function blah($x) :mixed{ return dict[foo() => $x]; }
function main() :mixed{ var_dump(blah('abc')); }

<<__EntryPoint>>
function main_array_subtype001() :mixed{
main();
}
