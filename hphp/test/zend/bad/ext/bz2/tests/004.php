<?php

$fd = bzopen(dirname(__FILE__)."/004_1.txt.bz2","r");
var_dump(bzerror($fd));
var_dump(bzerrstr($fd));
var_dump(bzerrno($fd));

$fd2 = bzopen(dirname(__FILE__)."/004_2.txt.bz2","r");
var_dump(bzerror($fd2));
var_dump(bzerrstr($fd2));
var_dump(bzerrno($fd2));

var_dump(bzread($fd, 10));
var_dump(bzerror($fd));
var_dump(bzerrstr($fd));
var_dump(bzerrno($fd));

var_dump(bzread($fd2, 10));
var_dump(bzerror($fd2));
var_dump(bzerrstr($fd2));
var_dump(bzerrno($fd2));

var_dump(bzread($fd));
var_dump(bzerror($fd));
var_dump(bzerrstr($fd));
var_dump(bzerrno($fd));

var_dump(bzread($fd2));
var_dump(bzerror($fd2));
var_dump(bzerrstr($fd2));
var_dump(bzerrno($fd2));

bzclose($fd2);
var_dump(bzread($fd2));
var_dump(bzerror($fd2));
var_dump(bzerrstr($fd2));
var_dump(bzerrno($fd2));

echo "Done\n";
?>