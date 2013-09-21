<?php
echo bin2hex(md5("", TRUE))."\n";
echo bin2hex(md5("a", TRUE))."\n";
echo bin2hex(md5("abc", TRUE))."\n";
echo bin2hex(md5("message digest", TRUE))."\n";
echo bin2hex(md5("abcdefghijklmnopqrstuvwxyz", TRUE))."\n";
echo bin2hex(md5("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", TRUE))."\n";
echo bin2hex(md5("12345678901234567890123456789012345678901234567890123456789012345678901234567890", TRUE))."\n";
?>