<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo() { return 123; }
function blah($x) { return dict[foo() => $x]; }
function main() { var_dump(blah('abc')); }
main();
