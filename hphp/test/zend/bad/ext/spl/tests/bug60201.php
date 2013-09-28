<?php

$method = new ReflectionMethod('SplFileObject', 'setCsvControl');
$params = $method->getParameters(); 
var_dump($params);

?>
===DONE===