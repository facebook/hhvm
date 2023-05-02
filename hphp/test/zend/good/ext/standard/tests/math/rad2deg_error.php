<?hh
/*
 * proto float rad2deg(float number)
*/
<<__EntryPoint>> function main(): void {
  $arg_0 = 1.0;
  $extra_arg = 1;

  echo "\nToo many arguments\n";
  try {
    var_dump(rad2deg($arg_0, $extra_arg));
  } catch (Exception $e) {
    echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
  }

  echo "\nToo few arguments\n";
  try {
    var_dump(rad2deg());
  } catch (Exception $e) {
    echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n";
  }
}
