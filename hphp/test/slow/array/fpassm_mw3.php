<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function byval($a) {}

function not_even_main() {
  byval($a[][0]);
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
