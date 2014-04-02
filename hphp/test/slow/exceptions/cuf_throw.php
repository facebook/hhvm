<?php

set_error_handler(function() { throw new Exception; });

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

main();
echo "Done\n";
