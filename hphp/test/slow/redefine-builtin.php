<?hh
function parse_str() { echo "parse_str()\n"; }
function main() { parse_str(); }

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_redefine_builtin() {
echo "Shouldn't see me\n";
main();
}
