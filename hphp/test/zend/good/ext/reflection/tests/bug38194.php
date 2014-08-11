<?php
class Object { }
  
$objectClass= new ReflectionClass('Object');
var_dump($objectClass->isSubclassOf($objectClass));
?>
