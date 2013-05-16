<?php

$file = dirname(__FILE__) .'/fgetcsv-test.csv';

$data = array(1, 2, 'foo', 'haha', array(4, 5, 6), 1.3, null);

$fp = fopen($file, 'w');

fputcsv($fp, $data);

var_dump($data);

@unlink($file);

?>