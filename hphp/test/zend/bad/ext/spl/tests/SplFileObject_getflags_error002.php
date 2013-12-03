<?php

file_put_contents('SplFileObject_getflags_error002.csv', 'eerste;tweede;derde');

$fo = new SplFileObject('SplFileObject_getflags_error002.csv');
$fo->setFlags(SplFileObject::READ_CSV);

$fo->getFlags('fake');

?>
<?php
unlink('SplFileObject_getflags_error002.csv');
?>