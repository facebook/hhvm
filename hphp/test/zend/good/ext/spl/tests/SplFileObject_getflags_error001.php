<?php

file_put_contents('SplFileObject_getflags_error001.csv', 'eerste;tweede;derde');


$fo = new SplFileObject('SplFileObject_getflags_error001.csv');
$fo->setFlags(SplFileObject::READ_CSV);

$fo->setFlags(SplFileObject::DROP_NEW_LINE);

var_dump($fo->getFlags());

?>
<?php
unlink('SplFileObject_getflags_error001.csv');
?>