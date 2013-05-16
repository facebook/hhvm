<?php

/* Testing fputcsv() to write to a file when default enclosure value and delimiter
   of two chars is provided */

echo "*** Testing fputcsv() : with default enclosure & delimiter of two chars ***\n";

$fo = new SplFileObject(__DIR__ . '/SplFileObject_fputcsv.csv', 'w');

var_dump($fo->fputcsv(array('water', 'fruit'), ',,', '"'));

unset($fo);

echo "Done\n";
?><?php
$file = __DIR__ . '/SplFileObject_fputcsv.csv';
unlink($file);
?>