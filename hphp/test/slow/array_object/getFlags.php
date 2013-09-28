<?php

// Array of available fruits
$fruits = array("lemons" => 1, "oranges" => 4, "bananas" => 5, "apples" => 10);

$fruitsArrayObject = new ArrayObject($fruits);

// Get the current flags
$flags = $fruitsArrayObject->getFlags();
var_dump($flags);

// Set new flags
$fruitsArrayObject->setFlags(ArrayObject::ARRAY_AS_PROPS);

// Get the new flags
$flags = $fruitsArrayObject->getFlags();
var_dump($flags);
