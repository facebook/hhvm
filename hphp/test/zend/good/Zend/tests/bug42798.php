<?php
function __autoload($className) {
    print "$className\n";
    exit();
}

function foo($c = ok::constant) {
}

foo();