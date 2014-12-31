<?php

function fn($reqParam, $optParam = null, ...$params) {
    var_dump($reqParam, $optParam, $params);
}
 
fn(1);
fn(1, 2);
fn(1, 2, 3);
fn(1, 2, 3, 4);
fn(1, 2, 3, 4, 5);

?>
