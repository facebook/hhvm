<?php

// Array of available fruits
$fruits = array(
  "lemons" => 1,
  "oranges" => 4,
  "bananas" => 5,
  "apples" => 10
);

$fruitsArrayObject = new ArrayObject($fruits);

// Try to use array key as property
var_dump($fruitsArrayObject->lemons);
// Set the flag so that the array keys can be used as properties
$fruitsArrayObject->setFlags(ArrayObject::ARRAY_AS_PROPS);
// Try it again
var_dump($fruitsArrayObject->lemons);
