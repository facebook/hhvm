<?php
$fp = fopen('SplFileObject__fgetcsv3.csv', 'w+');
fputcsv($fp, array(
	'field1',
	'field2',
	'field3',
	5
), '|');
fclose($fp);

$fo = new SplFileObject('SplFileObject__fgetcsv3.csv');
var_dump($fo->fgetcsv('invalid'));
?>
<?php error_reporting(0); ?>
<?php
unlink('SplFileObject__fgetcsv3.csv');
?>