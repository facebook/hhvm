<?php

$antonyms = [
    "good" => "bad",
    "happy" => "sad",
];

list(
    "good" => $good,
    "happy" => $happy
) = $antonyms;

var_dump($good, $happy);

echo PHP_EOL;

$antonyms = [
    "good" => "bad",
    "happy" => "sad",
];

list(
    "good" => $good,
    "happy" => $happy,
) = $antonyms;

var_dump($good, $happy);

?>
