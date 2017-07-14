<?php

$a = function(): \Iterator {
    yield 1;
    return;
};

foreach($a() as $value) {
    echo $value;
}

