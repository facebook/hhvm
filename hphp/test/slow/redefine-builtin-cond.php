<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

$already_ran = apc_fetch('already_ran');
if ($already_ran) {
  echo "Before redefine\n";
  function extract() { echo "extract()\n"; }
  function main() { extract(); }
  main();
} else {
  apc_store('already_ran', true);
}
