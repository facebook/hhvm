<?php
function foo() : callable {
    return function() {};
}

var_dump(foo());

