--TEST--
ripemd256 algorithm
--FILE--
<?php
echo hash('ripemd256', '') . "\n";
echo hash('ripemd256', 'a') . "\n";
echo hash('ripemd256', 'abc') . "\n";
echo hash('ripemd256', 'message digest') . "\n";
echo hash('ripemd256', 'abcdefghijklmnopqrstuvwxyz') . "\n";
echo hash('ripemd256', 'abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq') . "\n";
echo hash('ripemd256', 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') . "\n";
echo hash('ripemd256', '12345678901234567890123456789012345678901234567890123456789012345678901234567890') . "\n";
echo hash('ripemd256', str_repeat('a', 1000000)) . "\n";
