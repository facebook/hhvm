<?php
echo hash('ripemd128', '') . "\n";
echo hash('ripemd128', 'a') . "\n";
echo hash('ripemd128', 'abc') . "\n";
echo hash('ripemd128', 'message digest') . "\n";
echo hash('ripemd128', 'abcdefghijklmnopqrstuvwxyz') . "\n";
echo hash('ripemd128', 'abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq') . "\n";
echo hash('ripemd128', 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') . "\n";
echo hash('ripemd128', '12345678901234567890123456789012345678901234567890123456789012345678901234567890') . "\n";
echo hash('ripemd128', str_repeat('a', 1000000)) . "\n";