<?php
$fp = fopen('SplFileObject_fgetcsv_escape_default.csv', 'w+');
fwrite($fp, '"aa\"","bb","\"c"');
fclose($fp);

$fo = new SplFileObject('SplFileObject_fgetcsv_escape_default.csv');
var_dump($fo->fgetcsv());
?>
<?php
unlink('SplFileObject_fgetcsv_escape_default.csv');
?>