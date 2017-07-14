<?php

$results = [
    0 => 0,
    1 => 1,
    "" => ""
];

list(NULL => $NULL, 1.5 => $float, FALSE => $FALSE, TRUE => $TRUE) = $results;
var_dump($NULL, $float, $FALSE, $TRUE);

echo PHP_EOL;

list("0" => $zeroString, "1" => $oneString) = $results;
var_dump($zeroString, $oneString);

list(STDIN => $resource) = [];

?>
