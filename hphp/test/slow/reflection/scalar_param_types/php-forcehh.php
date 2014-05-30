<?php

function foo(int $bar, array $baz) {
};

$rp = new ReflectionParameter('foo', 'bar');
var_dump($rp->getClass());
var_dump($rp->getTypeHintText());

$rp = new ReflectionParameter('foo', 'baz');
var_dump($rp->getClass());
var_dump($rp->getTypeHintText());
