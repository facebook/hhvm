<?php

function gen1() {
    return [4, 5, 6];
}

function gen2() {
    yield from gen1();
}

$g = gen2();
foreach($g as $val) { var_dump($val); }
