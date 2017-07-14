<?php

function _array_(?array $v): ?array {
    return $v;
}

var_dump(_array_(null));
var_dump(_array_([]));

