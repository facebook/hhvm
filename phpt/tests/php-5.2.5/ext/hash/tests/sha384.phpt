--TEST--
sha384 algorithm
--FILE--
<?php
echo hash('sha384', '') . "\n";
echo hash('sha384', 'a') . "\n";
echo hash('sha384', '012345678901234567890123456789012345678901234567890123456789') . "\n";

/* FIPS-180 Vectors */
echo hash('sha384', 'abc') . "\n";
echo hash('sha384', 'abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu') . "\n";
echo hash('sha384', str_repeat('a', 1000000)) . "\n";
