--TEST--
md5 algorithm
--FILE--
<?php
echo hash('md5', '') . "\n";
echo hash('md5', 'a') . "\n";
echo hash('md5', '012345678901234567890123456789012345678901234567890123456789') . "\n";
echo hash('md5', str_repeat('a', 1000000)) . "\n";
