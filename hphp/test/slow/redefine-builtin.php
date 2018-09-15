<?hh
function extract() { echo "extract()\n"; }
function main() { extract(); }

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_redefine_builtin() {
echo "Shouldn't see me\n";
main();
}
