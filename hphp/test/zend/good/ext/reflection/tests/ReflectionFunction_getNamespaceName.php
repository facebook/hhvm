<?php
namespace A\B;
function foo() {}

$function = new \ReflectionFunction('sort');
var_dump($function->inNamespace());
var_dump($function->getName());
var_dump($function->getNamespaceName());
var_dump($function->getShortName());

$function = new \ReflectionFunction('A\\B\\foo');
var_dump($function->inNamespace());
var_dump($function->getName());
var_dump($function->getNamespaceName());
var_dump($function->getShortName());
?>
