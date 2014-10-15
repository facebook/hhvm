<?php
$fp = fopen('SplFileObject__fgetcsv8.csv', 'w+');
fwrite($fp, '"aaa","b""bb","ccc"');
fclose($fp);

$fo = new SplFileObject('SplFileObject__fgetcsv8.csv');
var_dump($fo->fgetcsv(',', '"', 'invalid'));
?>
<?php error_reporting(0); ?>
<?php
unlink('SplFileObject__fgetcsv8.csv');
?>