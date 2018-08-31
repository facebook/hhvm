<?php


<<__EntryPoint>>
function main_reflection_class_extension() {
$ref = new ReflectionClass('ReflectionClass');
var_dump($ref->getExtension() instanceof ReflectionExtension);
var_dump(is_string($ref->getExtensionName()));
}
