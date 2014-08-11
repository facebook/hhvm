<?php

var_dump(new ReflectionMethod());
var_dump(new ReflectionMethod('a', 'b', 'c'));

class C {
    public function f() {}
}

$rm = new ReflectionMethod('C', 'f');

var_dump($rm->isFinal(1));
var_dump($rm->isAbstract(1));
var_dump($rm->isPrivate(1));
var_dump($rm->isProtected(1));
var_dump($rm->isPublic(1));
var_dump($rm->isStatic(1));
var_dump($rm->isConstructor(1));
var_dump($rm->isDestructor(1));
var_dump($rm->getModifiers(1));
var_dump($rm->isInternal(1));
var_dump($rm->isUserDefined(1));
var_dump($rm->getFileName(1));
var_dump($rm->getStartLine(1));
var_dump($rm->getEndLine(1));
var_dump($rm->getStaticVariables(1));
var_dump($rm->getName(1));


?>
