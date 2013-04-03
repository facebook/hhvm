<?php
ini_set('display_errors',true);
$mb=148;
$var = '';
for ($i=0; $i<=$mb; $i++) {
        $var.= str_repeat('a',1*1024*1024);
}
?>