<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

// Make sure we're tolerant of code that swallows typehint failures
function error_handler() {
  var_dump(func_get_args());
  return true;
}

function main(integer $foo) {
  echo "in main\n";
}
set_error_handler('error_handler');
main(0.0);
echo "done with main\n";
