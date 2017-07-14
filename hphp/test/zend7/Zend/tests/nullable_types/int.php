<?php

function _int_(?int $v): ?int {
    return $v;
}

var_dump(_int_(null));
var_dump(_int_(1));

