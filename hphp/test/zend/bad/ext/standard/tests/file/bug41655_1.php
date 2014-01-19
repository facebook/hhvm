<?php
ini_set('open_basedir', /tmp);

$a=glob("./*.jpeg");
var_dump($a);
echo "Done\n";
?>