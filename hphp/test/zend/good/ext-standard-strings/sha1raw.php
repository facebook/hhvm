<?php
echo bin2hex(sha1("abc", TRUE))."\n";
echo bin2hex(sha1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", TRUE))."\n";
echo bin2hex(sha1("a", TRUE))."\n";
echo bin2hex(sha1("0123456701234567012345670123456701234567012345670123456701234567", TRUE))."\n";
?>