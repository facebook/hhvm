<?hh

function testPointerModifications() {
  $array = array(1, 2, 3);
  next(inout $array);
  $arrayObject = new ArrayObject($array);

  reset(inout $arrayObject);
  echo "After reset current of " . get_class($arrayObject) . " value is: ";
  echo current($arrayObject), "\n";

  next(inout $arrayObject);
  echo "Next value of " . get_class($arrayObject) . " is: ";
  echo current($arrayObject), "\n";

  echo "Looking back to real array, current value is: ";
  echo current($array), "\n";

  echo "Key of current value of " . get_class($arrayObject) . " is: ";
  echo key($arrayObject), "\n";

  echo "Previous value of " . get_class($arrayObject) . " is: ";
  echo prev(inout $arrayObject), "\n";

  $keyValuePair = each(inout $arrayObject);
  echo "Current key-value pair of " . get_class($arrayObject) . " is: ";
  echo "(", $keyValuePair[0], ', ', $keyValuePair[1], ")\n";

  end(inout $arrayObject);
  echo "Last value of " . get_class($arrayObject) . " is: ";
  echo current($arrayObject), "\n";

  reset(inout $arrayObject);
  next(inout $arrayObject);
  foreach($arrayObject as $key => $value) {
    echo "next/reset etc have no impact on using arrayObject as iterator, ";
    echo "proof: ", $value, "\n";
    break;
  }

  $arrayObject->setFlags(ArrayObject::STD_PROP_LIST);
  echo "current value after setting STD_PROP_LIST: ";
  echo var_export(current($arrayObject), true), "\n";

  foreach(array('each', 'prev', 'next', 'reset') as $op) {
    echo "$op with STD_PROP_LIST enabled: ";
    echo var_export($op(inout $arrayObject), true), "\n";
  }
  foreach(array('current', 'key') as $op) {
    echo "$op with STD_PROP_LIST enabled: ";
    echo var_export($op($arrayObject), true), "\n";
  }

  $arrayObject->setFlags(ArrayObject::ARRAY_AS_PROPS);
  echo "Disabling STD_PROP_LIST and checking current value: ";
  echo current($arrayObject), "\n";
}

<<__EntryPoint>>
function main_pointer_modifications() {
testPointerModifications();
}
