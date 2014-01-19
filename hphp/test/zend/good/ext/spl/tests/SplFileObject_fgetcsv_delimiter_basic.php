<?php
$fp = fopen('SplFileObject_fgetcsv_delimiter_basic.csv', 'w+');
fputcsv($fp, array(
	'field1',
	'field2',
	'field3',
	5
), '|');
fclose($fp);

$fo = new SplFileObject('SplFileObject_fgetcsv_delimiter_basic.csv');
var_dump($fo->fgetcsv('|'));
?>
<?php
unlink('SplFileObject_fgetcsv_delimiter_basic.csv');
?>