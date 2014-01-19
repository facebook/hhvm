<?php
echo hash('crc32', ''), "\n";
echo hash('crc32', 'a'), "\n";
echo hash('crc32', 'abc'), "\n";
echo hash('crc32', 'message digest'), "\n";
echo hash('crc32', 'abcdefghijklmnopqrstuvwxyz'), "\n";
echo hash('crc32', 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789'), "\n";
echo hash('crc32', '12345678901234567890123456789012345678901234567890123456789012345678901234567890'), "\n";
echo hash('crc32b', ''), "\n";
echo hash('crc32b', 'a'), "\n";
echo hash('crc32b', 'abc'), "\n";
echo hash('crc32b', 'message digest'), "\n";
echo hash('crc32b', 'abcdefghijklmnopqrstuvwxyz'), "\n";
echo hash('crc32b', 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789'), "\n";
echo hash('crc32b', '12345678901234567890123456789012345678901234567890123456789012345678901234567890'), "\n";
?>