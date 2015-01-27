<?php

function foo() {
    $t = "this";
    $$t = 5;
    extract(array("this" => "foo"));
    var_dump($this);
}

foo();
