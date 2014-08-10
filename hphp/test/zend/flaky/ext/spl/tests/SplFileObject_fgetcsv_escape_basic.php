<?php
$fp = fopen('SplFileObject_fgetcsv_escape_basic.csv', 'w+');
fwrite($fp, '"aaa","b""bb","ccc"');
fclose($fp);

$fo = new SplFileObject('SplFileObject_fgetcsv_escape_basic.csv');
var_dump($fo->fgetcsv(',', '"', '"'));
?>
<?php
unlink('SplFileObject_fgetcsv_escape_basic.csv');
?>