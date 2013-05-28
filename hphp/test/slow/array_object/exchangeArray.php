<?php

// Array of available fruits
$fruits = array("lemons" => 1, "oranges" => 4, "bananas" => 5, "apples" => 10);
// Array of locations in Europe
$locations = array('Amsterdam', 'Paris', 'London');

$fruitsArrayObject = new ArrayObject($fruits);

// Now exchange fruits for locations
$old = $fruitsArrayObject->exchangeArray($locations);
print_r($old);
print_r($fruitsArrayObject);
