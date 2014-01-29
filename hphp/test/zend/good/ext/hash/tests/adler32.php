<?php
echo hash('adler32', ''), "\n";
echo hash('adler32', 'a'), "\n";
echo hash('adler32', 'abc'), "\n";
echo hash('adler32', 'message digest'), "\n";
echo hash('adler32', 'abcdefghijklmnopqrstuvwxyz'), "\n";
echo hash('adler32', 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789'), "\n";
echo hash('adler32', '12345678901234567890123456789012345678901234567890123456789012345678901234567890'), "\n";
?>