<?php

function foo() {
    $d = new DOMDocument;
    $e = $d->createElement('foo');
    $f = $d->createElement('bar');
    $e->appendChild($f);
    return $f;
}

$c = new DOMDocument;
var_dump($c->appendChild(foo()));
