<?php

require_once('test_base.inc');

function headerTestController($serverPort) {
  $args = array('Authorization' => 'foo');
  var_dump(request(php_uname('n'), $serverPort, "test_headers.php",
                  [], [], $args));
}

runTest("headerTestController");
