<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  $attached = hphp_debugger_attached();
  var_dump($attached);
  $info = hphp_get_debugger_info();
  var_dump($info);
}

main();
