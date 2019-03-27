<?php

function by_ref(&$ref) {}

class aclass {
  const myConst = "hello";
}

echo "\nAttempting to create a reference to a class constant - should be parse error.\n";
by_ref(&aclass::myConst);
