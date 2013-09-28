<?php
$file = dirname(__FILE__).'/bug40501.csv';

$h = fopen($file, 'r');
$data = fgetcsv($h, NULL, ',', '"', '"');
fclose($h);

var_dump($data);
?>