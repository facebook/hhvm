--TEST--
md2 algorithm
--FILE--
<?php
echo hash('md2', '') . "\n";
echo hash('md2', 'a') . "\n";
echo hash('md2', 'abc') . "\n";
echo hash('md2', 'message digest') . "\n";
echo hash('md2', 'abcdefghijklmnopqrstuvwxyz') . "\n";
echo hash('md2', 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') . "\n";
echo hash('md2', '12345678901234567890123456789012345678901234567890123456789012345678901234567890') . "\n";
