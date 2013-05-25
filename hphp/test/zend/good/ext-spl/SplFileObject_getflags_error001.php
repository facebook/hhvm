<?php

file_put_contents('testdata.csv', 'eerste;tweede;derde');


$fo = new SplFileObject('testdata.csv');
$fo->setFlags(SplFileObject::READ_CSV);

$fo->setFlags(SplFileObject::DROP_NEW_LINE);

var_dump($fo->getFlags());

?><?php
unlink('testdata.csv');
?>