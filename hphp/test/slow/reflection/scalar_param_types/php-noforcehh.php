<?php

function foo(int $bar, array $baz) {
};

$rp = new ReflectionParameter('foo', 'bar');
try {
var_dump($rp->getClass());
} catch (Exception $e) {
  var_dump((string) $e);
}
var_dump($rp->getTypeHintText());

$rp = new ReflectionParameter('foo', 'baz');
try {
var_dump($rp->getClass());
} catch (Exception $e) {
  var_dump((string) $e);
}
var_dump($rp->getTypeHintText());
