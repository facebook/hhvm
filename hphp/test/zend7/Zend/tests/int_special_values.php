<?php
$values = [
    0.0,
    INF,
    -INF,
    1 / INF,
    -1 / INF, // Negative zero,
    NAN
];

foreach($values as $value) {
    var_dump($value);
    var_dump((int)$value);
    echo PHP_EOL;
}
?>
