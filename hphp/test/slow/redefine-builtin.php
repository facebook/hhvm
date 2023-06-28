<?hh
function parse_str() :mixed{ echo "parse_str()\n"; }
function main() :mixed{ parse_str(); }

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_redefine_builtin() :mixed{
echo "Shouldn't see me\n";
main();
}
