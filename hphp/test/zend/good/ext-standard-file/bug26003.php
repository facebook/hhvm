<?php
$fp = fopen(dirname(__FILE__).'/test3.csv', 'r');
var_dump(fgetcsv($fp, 4096));
?>