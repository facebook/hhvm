<?php

function aux($fun) {
  $func = new ReflectionFunction($fun);
  $parameters = $func->getParameters();
  foreach($parameters as $parameter) {
    echo "Name: ", $parameter->getName(), "\n";
    echo "Is passed by reference: ", $parameter->isPassedByReference()?"yes":"no", "\n";
    echo "Can be passed by value: ", $parameter->canBePassedByValue()?"yes":"no", "\n";
    echo "\n";
  }
}

function ufunc(&$arg1, $arg2) {}
aux('ufunc');
