<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo1() { return "key"; }
function foo2() { return "value"; }
function blah() { return dict[foo1() => foo2()]; }
function main() { var_dump(blah()); }

<<__EntryPoint>>
function main_array_subtype003() {
main();
}
