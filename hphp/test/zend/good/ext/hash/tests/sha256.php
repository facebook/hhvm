<?php
echo hash('sha256', '') . "\n";
echo hash('sha256', 'a') . "\n";
echo hash('sha256', '012345678901234567890123456789012345678901234567890123456789') . "\n";

/* FIPS-180 Vectors */
echo hash('sha256', 'abc') . "\n";
echo hash('sha256', 'abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq') . "\n";
echo hash('sha256', str_repeat('a', 1000000)) . "\n";