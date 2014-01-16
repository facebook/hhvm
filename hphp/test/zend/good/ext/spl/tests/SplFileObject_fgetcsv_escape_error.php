<?php
$fp = fopen('SplFileObject_fgetcsv_escape_error.csv', 'w+');
fwrite($fp, '"aaa","b""bb","ccc"');
fclose($fp);

$fo = new SplFileObject('SplFileObject_fgetcsv_escape_error.csv');
var_dump($fo->fgetcsv(',', '"', 'invalid'));
?>
<?php
unlink('SplFileObject_fgetcsv_escape_error.csv');
?>