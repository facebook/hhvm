<?hh

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_redefine_builtin_cond() {
$already_ran = apc_fetch('already_ran');
if ($already_ran) {
  echo "Before redefine\n";
  function parse_str() { echo "parse_str()\n"; }
  function main() { parse_str(); }
  main();
} else {
  apc_store('already_ran', true);
}
}
