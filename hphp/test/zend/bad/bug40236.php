<?php
$php = getenv('TEST_PHP_EXECUTABLE');
$cmd = "\"$php\" -n -d memory_limit=4M -a \"".dirname(__FILE__)."\"/bug40236.inc";
echo `$cmd`;
?>