<?php
$a = array(1,2,3);
$data = array($a);
$data = array_map('current', $data);
var_dump($data);
?>
