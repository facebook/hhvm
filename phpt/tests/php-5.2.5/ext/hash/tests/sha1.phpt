--TEST--
sha1 algorithm
--FILE--
<?php
echo hash('sha1', '') . "\n";
echo hash('sha1', 'a') . "\n";
echo hash('sha1', '012345678901234567890123456789012345678901234567890123456789') . "\n";

/* FIPS-180 Vectors */
echo hash('sha1', 'abc') . "\n";
echo hash('sha1', 'abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq') . "\n";
echo hash('sha1', str_repeat('a', 1000000)) . "\n";
