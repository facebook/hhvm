<?php

file_put_contents('testdata.csv', 'eerste;tweede;derde');

$fo = new SplFileObject('testdata.csv');
$fo->setFlags(SplFileObject::READ_CSV);

$fo->getFlags('fake');

?><?php
unlink('testdata.csv');
?>