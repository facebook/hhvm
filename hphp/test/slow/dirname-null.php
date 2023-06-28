<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() :mixed{
  var_dump(pathinfo("\x00"));
}

<<__EntryPoint>>
function main_dirname_null() :mixed{
main();
}
