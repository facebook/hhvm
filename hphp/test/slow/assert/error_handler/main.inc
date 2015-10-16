<?php

assert_options(ASSERT_ACTIVE, 1);

set_error_handler(function ($errno, $errstr) {
  printf("Error handler: code: %d message: %s\n", $errno, $errstr);
});

echo "begin\n";
assert(false, 'lolol');
echo "end\n";
