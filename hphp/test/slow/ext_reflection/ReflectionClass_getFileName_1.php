<?php

class UserClass {
}

$class = new ReflectionClass('UserClass');
$parts = split("/", $class->getFileName());

var_dump($parts[count($parts) - 1]);
