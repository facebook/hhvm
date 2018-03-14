<?php

function test_fastcgi() {
  echo "before\n";
  fastcgi_finish_request();
  echo "after\n";
}

if ($_GET['error']) {
  register_shutdown_function('test_fastcgi');
  func_not_exists();
} else {
  test_fastcgi();
}
