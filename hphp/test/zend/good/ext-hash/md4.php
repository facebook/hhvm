<?php
/* RFC 1320 vectors */
echo hash('md4', '') . "\n";
echo hash('md4', 'a') . "\n";
echo hash('md4', 'abc') . "\n";
echo hash('md4', 'message digest') . "\n";
echo hash('md4', 'abcdefghijklmnopqrstuvwxyz') . "\n";
echo hash('md4', 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') . "\n";
echo hash('md4', '12345678901234567890123456789012345678901234567890123456789012345678901234567890') . "\n";