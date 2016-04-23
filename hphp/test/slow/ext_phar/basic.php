<?php

echo ".phar:\n";
include __DIR__."/basic.phar";

echo "\n.phar with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic.phar";

// TODO: not working somewhere deep inside HHVM; if you know how to fix - do it
//echo "\n.phar.gz:\n";
//include __DIR__."/basic.phar.gz";

echo "\n.phar.gz with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic.phar.gz";

// TODO: not working somewhere deep inside HHVM; if you know how to fix - do it
//echo "\n.phar.bz2:\n";
//include __DIR__."/basic.phar.bz2";

echo "\n.phar.bz2 with phar:// prefix:\n";
include 'phar://'.__DIR__."/basic.phar.bz2";
