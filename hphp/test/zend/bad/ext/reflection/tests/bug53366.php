<?php

class UserClass {
}

$myClass = new UserClass;
$myClass->id = 1000;

$reflect = new ReflectionObject($myClass);

var_dump($reflect->getProperty('id'));
var_dump($reflect->getProperty('id')->getValue($myClass));

?>
