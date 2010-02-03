--TEST--
ripemd160 algorithm
--FILE--
<?php
echo hash('ripemd160', '') . "\n";
echo hash('ripemd160', 'a') . "\n";
echo hash('ripemd160', 'abc') . "\n";
echo hash('ripemd160', 'message digest') . "\n";
echo hash('ripemd160', 'abcdefghijklmnopqrstuvwxyz') . "\n";
echo hash('ripemd160', 'abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq') . "\n";
echo hash('ripemd160', 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') . "\n";
echo hash('ripemd160', '12345678901234567890123456789012345678901234567890123456789012345678901234567890') . "\n";
echo hash('ripemd160', str_repeat('a', 1000000)) . "\n";
