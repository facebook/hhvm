<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo1() { return 0; }
function foo2() { return 1; }
function foo3() { return 2; }
function blah($x) { return dict[foo1() => $x, foo2() => $x, foo3() => $x]; }
function main() { var_dump(blah('abc')); }

<<__EntryPoint>>
function main_array_subtype002() {
main();
}
