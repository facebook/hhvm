<?php

chdir(dirname(__FILE__));
setlocale(LC_ALL, 'en_US.UTF-8');
bindtextdomain ("messages", "./locale");
echo textdomain('test'), "\n";
echo textdomain(null), "\n";
echo textdomain('foo'), "\n";
?>