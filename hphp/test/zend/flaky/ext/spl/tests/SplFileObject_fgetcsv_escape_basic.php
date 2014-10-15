<?php
$fp = fopen('SplFileObject__fgetcsv6.csv', 'w+');
fwrite($fp, '"aaa","b""bb","ccc"');
fclose($fp);

$fo = new SplFileObject('SplFileObject__fgetcsv6.csv');
var_dump($fo->fgetcsv(',', '"', '"'));
?>
<?php error_reporting(0); ?>
<?php
unlink('SplFileObject__fgetcsv6.csv');
?>