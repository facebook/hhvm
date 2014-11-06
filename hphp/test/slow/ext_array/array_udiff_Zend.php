<?php

// See context: https://github.com/facebook/hhvm/issues/3653

$items = array(new \stdClass(), new \stdClass());

$udiff = array_udiff($items, $items, function ($a, $b) {
    return $a === $b;
});

var_dump($udiff);
