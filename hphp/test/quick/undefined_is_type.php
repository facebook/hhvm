<?hh

<<__EntryPoint>> function foo(): void {
  // Force all the types to be in so these don't get constant folded.
  if (\HH\global_isset('a')) $a = 1;
  if (\HH\global_isset('b')) $a = 1.2;
  if (\HH\global_isset('c')) $a = '1';
  if (\HH\global_isset('d')) $a = new stdclass;
  if (\HH\global_isset('e')) $a = varray[];
  if (\HH\global_isset('f')) $a = false;
  if (\HH\global_isset('g')) $a = null;

  echo "set:     ", isset($a)      . "\n";
  echo "nul:     ", is_null($a)    . "\n";
  echo "str:     ", is_string($a)  . "\n";
  echo "obj:     ", is_object($a)  . "\n";
  echo "arr:     ", is_array($a)   . "\n";
  echo "int:     ", is_int($a)     . "\n";
  echo "integer: ", is_integer($a) . "\n";
  echo "long:    ", is_long($a)    . "\n";
  echo "real:    ", is_real($a)    . "\n";
  echo "double:  ", is_double($a)  . "\n";
  echo "float:   ", is_float($a)   . "\n";
  echo "bool:    ", is_bool($a)    . "\n";
}
