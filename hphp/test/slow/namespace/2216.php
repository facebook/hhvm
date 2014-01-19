<?php

namespace my\name;
class MyClass {
}
function myfunction() {
}
const MYCONST = 123;

$a = new MyClass;
 var_dump(get_class($a));
$c = new \my\name\MyClass;
 var_dump(get_class($a));
$a = strlen('hi');
 var_dump($a);
$d = namespace\MYCONST;
 var_dump($d);
$d = __NAMESPACE__ . '\MYCONST';
 var_dump(constant($d));
var_dump(defined('MYCONST'));
