<?php
echo hash('sha224', '') . "\n";
echo hash('sha224', 'a') . "\n";
echo hash('sha224', '012345678901234567890123456789012345678901234567890123456789') . "\n";

/* FIPS-180 Vectors */
echo hash('sha224', 'abc') . "\n";
echo hash('sha224', 'abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq') . "\n";
echo hash('sha224', str_repeat('a', 1000000)) . "\n";