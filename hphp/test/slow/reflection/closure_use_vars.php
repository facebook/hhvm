<?php

$refVariable = false;
$refName = 'refVariable';
$callback = function() use (&$refVariable) { var_dump($refVariable); };
$function = new \ReflectionFunction($callback);
$staticVariables = $function->getStaticVariables();
var_dump($staticVariables);
var_dump(array_key_exists($refName, $staticVariables));
var_dump($refVariable);
$staticVariables[$refName] = true;
var_dump($refVariable);
$callback();
$refVariable = 'foo';
var_dump($staticVariables[$refName]);
$callback();
