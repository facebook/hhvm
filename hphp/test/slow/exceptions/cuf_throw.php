<?php

class X {
  function __destruct() {
    echo __METHOD__, "\n";
  }
}

function test($x) {
  call_user_func(array($x, "FOO"));
}

function main() {
  try {
    test(new X);
  } catch (Exception $e) {
    echo "Exception\n";
  }
}


<<__EntryPoint>>
function main_cuf_throw() {
set_error_handler(function() { throw new Exception; });

main();
echo "Done\n";
}
