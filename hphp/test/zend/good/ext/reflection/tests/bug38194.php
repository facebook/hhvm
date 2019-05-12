<?php
class Object { }
<<__EntryPoint>> function main() {
$objectClass= new ReflectionClass('Object');
var_dump($objectClass->isSubclassOf($objectClass));
}
