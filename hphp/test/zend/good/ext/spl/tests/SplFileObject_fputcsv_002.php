<?php
$fo = new SplFileObject(__DIR__ . '/SplFileObject_fputcsv1.csv', 'w');

$data = array(1, 2, 'foo', 'haha', array(4, 5, 6), 1.3, null);

$fo->fputcsv($data);

var_dump($data);
error_reporting(0);
$file = __DIR__ . '/SplFileObject_fputcsv1.csv';
unlink($file);
