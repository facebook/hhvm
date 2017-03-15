<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

echo "Shouldn't see me\n";
function extract() { echo "extract()\n"; }
function main() { extract(); }
main();
