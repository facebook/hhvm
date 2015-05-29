<?php

function testPointerModifications() {
  $array = array(1, 2, 3);
  $arrayObject = new ArrayObject($array);

  reset($arrayObject);
  echo "After reset current of " . get_class($arrayObject) . " value is: ";
  echo current($arrayObject), "\n";

  next($arrayObject);
  echo "Next value of " . get_class($arrayObject) . " is: ";
  echo current($arrayObject), "\n";

  echo "Looking back to real array, current value is: ";
  echo current($array), "\n";

  echo "Key of current value of " . get_class($arrayObject) . " is: ";
  echo key($arrayObject), "\n";

  echo "Previous value of " . get_class($arrayObject) . " is: ";
  echo prev($arrayObject), "\n";

  $keyValuePair = each($arrayObject);
  echo "Current key-value pair of " . get_class($arrayObject) . " is: ";
  echo "(", $keyValuePair[0], ', ', $keyValuePair[1], ")\n";

  end($arrayObject);
  echo "Last value of " . get_class($arrayObject) . " is: ";
  echo current($arrayObject), "\n";
}
testPointerModifications();

