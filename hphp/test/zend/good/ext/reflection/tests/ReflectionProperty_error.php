<?php

class C {
    public static $p;
}

var_dump(new ReflectionProperty());
var_dump(new ReflectionProperty('C::p'));
var_dump(new ReflectionProperty('C', 'p', 'x'));
$rp = new ReflectionProperty('C', 'p');
var_dump($rp->getName(1));
var_dump($rp->isPrivate(1));
var_dump($rp->isProtected(1));
var_dump($rp->isPublic(1));
var_dump($rp->isStatic(1));
var_dump($rp->getModifiers(1));
var_dump($rp->isDefault(1));

?>
