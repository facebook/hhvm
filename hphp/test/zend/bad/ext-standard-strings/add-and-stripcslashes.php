<?php
echo addcslashes("", "")."\n";
echo addcslashes("", "burp")."\n";
echo addcslashes("kaboemkara!", "")."\n";
echo addcslashes("foobarbaz", 'bar')."\n";
echo addcslashes('foo[ ]', 'A..z')."\n";
echo @addcslashes("zoo['.']", 'z..A')."\n";
echo addcslashes('abcdefghijklmnopqrstuvwxyz', "a\145..\160z")."\n";
echo "\n\r" == stripcslashes('\n\r'),"\n";
echo stripcslashes('\065\x64')."\n";
echo stripcslashes('')."\n";
?>