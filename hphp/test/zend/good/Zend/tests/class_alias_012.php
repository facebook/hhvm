<?php

namespace test\baz;

class foo { 
}

new foo;

class_alias(__NAMESPACE__ .'\foo', __NAMESPACE__ .'\T');

var_dump(new foo);
var_dump(new T);

$var = __NAMESPACE__ .'\foo';
var_dump(new $var);

$var = __NAMESPACE__ .'\T';
var_dump(new $var);

?>