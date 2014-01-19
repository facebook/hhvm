<?php

// Array of available fruits
$fruits = array("lemons" => 1, "oranges" => 4, "bananas" => 5, "apples" => 10);

$fruitsArrayObject = new ArrayObject($fruits);
$fruitsArrayObject['pears'] = 4;

// create a copy of the array
$copy = $fruitsArrayObject->getArrayCopy();
print_r($copy);
