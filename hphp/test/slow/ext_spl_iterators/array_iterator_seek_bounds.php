<?php

$array = [1];
$i = new ArrayIterator($array);

try {
    $i->seek(-1);
} catch (Exception $e) {
    $class = get_class($e);
    echo "{$class}: {$e->getMessage()}\n";
}
try {
    $i->seek(1);
} catch (Exception $e) {
    $class = get_class($e);
    echo "{$class}: {$e->getMessage()}\n";
}
