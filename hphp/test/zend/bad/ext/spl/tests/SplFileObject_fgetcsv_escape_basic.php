<?php
$fp = fopen('SplFileObject__fgetcsv.csv', 'w+');
fwrite($fp, '"aaa","b""bb","ccc"');
fclose($fp);

$fo = new SplFileObject('SplFileObject__fgetcsv.csv');
var_dump($fo->fgetcsv(',', '"', '"'));
?>
<?php
unlink('SplFileObject__fgetcsv.csv');
?>