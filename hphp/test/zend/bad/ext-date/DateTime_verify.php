<?php

echo "*** Verify DateTime class ***\n";

echo "Verify DateTime class registered OK\n";
$class = new ReflectionClass('DateTime');
var_dump($class);

echo "..and get names of all its methods\n";
$methods = $class->getMethods(); 
var_dump($methods);

echo "..and get names of all its class constants\n"; 
$constants = $class->getConstants();
var_dump($constants);

?>
===DONE===