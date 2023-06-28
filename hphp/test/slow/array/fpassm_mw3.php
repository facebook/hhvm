<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function byval($a) :mixed{}

function not_even_main() :mixed{
  byval($a[][0]);
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
