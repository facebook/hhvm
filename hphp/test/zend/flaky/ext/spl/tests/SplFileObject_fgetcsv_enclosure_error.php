<?php
$fp = fopen('SplFileObject_fgetcsv_enclosure_error.csv', 'w+');
fputcsv($fp, array(
	'field1',
	'field2',
	'field3',
	5
), ',', '"');
fclose($fp);

$fo = new SplFileObject('SplFileObject_fgetcsv_enclosure_error.csv');
var_dump($fo->fgetcsv(',', 'invalid'));
?>
<?php
unlink('SplFileObject_fgetcsv_enclosure_error.csv');
?>