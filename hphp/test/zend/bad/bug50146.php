<?php

$obj = function(){};

var_dump(property_exists($obj,'foo'));

$ref = new ReflectionObject($obj);
var_dump($ref->hasProperty('b'));

var_dump(isset($obj->a));

?>