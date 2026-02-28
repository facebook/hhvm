<?hh

<<__EntryPoint>>
function foo(): void {
  // Force all the types to be in so these don't get constant folded.
  if (\HH\global_isset('a')) $a = 1;
  if (\HH\global_isset('b')) $a = 1.2;
  if (\HH\global_isset('c')) $a = '1';
  if (\HH\global_isset('d')) $a = new stdClass;
  if (\HH\global_isset('e')) $a = vec[];
  if (\HH\global_isset('f')) $a = false;
  if (\HH\global_isset('g')) $a = null;

  echo "set:     ", (string)(isset($a))."\n";
  try {
    echo "nul:     ", is_null($a)."\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "str:     ", is_string($a)."\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "obj:     ", is_object($a)."\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "arr:     ", is_array($a)."\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "int:     ", is_int($a)."\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "integer: ", is_integer($a)."\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "long:    ", is_long($a)."\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "real:    ", is_real($a)."\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "double:  ", is_double($a)."\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "float:   ", is_float($a)."\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    echo "bool:    ", is_bool($a)."\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }


}
