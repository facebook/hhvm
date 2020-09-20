<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main() {
  var_dump(mb_detect_order("UTF-8,\x2c"));
}
