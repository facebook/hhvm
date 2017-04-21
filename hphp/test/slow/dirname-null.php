<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  var_dump(pathinfo("\x00"));
}
main();
