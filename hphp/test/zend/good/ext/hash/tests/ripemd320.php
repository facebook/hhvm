<?php
echo hash('ripemd320', '') . "\n";
echo hash('ripemd320', 'a') . "\n";
echo hash('ripemd320', 'abc') . "\n";
echo hash('ripemd320', 'message digest') . "\n";
echo hash('ripemd320', 'abcdefghijklmnopqrstuvwxyz') . "\n";
echo hash('ripemd320', 'abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq') . "\n";
echo hash('ripemd320', 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') . "\n";
echo hash('ripemd320', '12345678901234567890123456789012345678901234567890123456789012345678901234567890') . "\n";
echo hash('ripemd320', str_repeat('a', 1000000)) . "\n";