<?php

// Custom ArrayIterator (inherits from ArrayIterator)
class MyArrayIterator extends ArrayIterator {
    // custom implementation
}

// Array of available fruits
$fruits = array("lemons" => 1, "oranges" => 4, "bananas" => 5, "apples" => 10);

$fruitsArrayObject = new ArrayObject($fruits);

// Get the current class name
$className = $fruitsArrayObject->getIteratorClass();
var_dump($className);

// Set new classname
$fruitsArrayObject->setIteratorClass('MyArrayIterator');

// Get the new iterator classname
$className = $fruitsArrayObject->getIteratorClass();
var_dump($className);
