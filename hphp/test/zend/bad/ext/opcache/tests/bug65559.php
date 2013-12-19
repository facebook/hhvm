<?php
$file =  __DIR__ . "/bug6559.inc.php";
file_put_contents($file, '<?php return 1;');
$var = include $file;
var_dump($var);
file_put_contents($file, '<?php return 2;');
$var = include $file;
var_dump($var);
@unlink($file);
?>