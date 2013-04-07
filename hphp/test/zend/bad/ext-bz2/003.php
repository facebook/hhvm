<?php

$fd = bzopen(dirname(__FILE__)."/003.txt.bz2","r");
var_dump(bzread());
var_dump(bzread($fd, 1 ,0));
var_dump(bzread($fd, 0));
var_dump(bzread($fd, -10));
var_dump(bzread($fd, 1));
var_dump(bzread($fd, 2));
var_dump(bzread($fd, 100000));

echo "Done\n";
?>