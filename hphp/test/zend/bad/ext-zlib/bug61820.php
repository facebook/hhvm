<?php
ob_start('ob_gzhandler');

echo "Hi there.\n";
ob_flush();
flush();

echo "This is confusing...\n";
ob_flush();
flush();
?>
DONE