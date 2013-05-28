<?php

class C {
public function mE() {
echo 'fail';
}
}
$ref = new ReflectionClass('C');
var_dump($ref->hasMethod('mE'));
var_dump($ref->hasMethod('me'));
$m = $ref->getMethod('me');
var_dump($m->getName());
