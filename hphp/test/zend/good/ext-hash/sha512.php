<?php
echo hash('sha512', '') . "\n";
echo hash('sha512', 'a') . "\n";
echo hash('sha512', '012345678901234567890123456789012345678901234567890123456789') . "\n";

/* FIPS-180 Vectors */
echo hash('sha512', 'abc') . "\n";
echo hash('sha512', 'abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu') . "\n";
echo hash('sha512', str_repeat('a', 1000000)) . "\n";