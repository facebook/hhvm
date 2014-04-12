<?php

$ref = new ReflectionClass('ReflectionClass');
var_dump($ref->getExtension() instanceof ReflectionExtension);
var_dump(is_string($ref->getExtensionName()));
