<?php

$class  = new ReflectionClass('ReflectionMethod');
$method = $class->getMethod('setAccessible');
$params = $method->getParameters();
var_dump($params[0]->getClass());

$res = new ReflectionMethod( new ArrayObject(), 'setFlags' );
$res = $res->getParameters();
var_dump( $res[0]->getClass() );
