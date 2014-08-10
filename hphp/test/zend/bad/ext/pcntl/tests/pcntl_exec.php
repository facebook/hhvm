<?php
echo "ok\n";
pcntl_exec(getenv("TEST_PHP_EXECUTABLE"), ['-n']);
echo "nok\n";
?>