<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  echo "xdebug.enable = " . (ini_get("xdebug.enable") ? "true" : "false");
  trigger_error("This test should not print a stack.", E_USER_WARNING);
}

main();
