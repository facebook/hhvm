<?hh
/*
 * proto bool ob_start([ string|array user_function [, int chunk_size [, bool erase]]])
 * Function is implemented in main/output.c
*/
// In HEAD, $chunk_size value of 1 should not have any special behaviour (http://marc.info/?l=php-internals&m=123476465621346&w=2).
abstract final class ObStartBasic {
  public static $callback_invocations = 0;
}
function callback($string) {

  ObStartBasic::$callback_invocations++;
    $len = strlen($string);
    return "f[call:".ObStartBasic::$callback_invocations."; len:$len]$string\n";
}
<<__EntryPoint>> function main(): void {
for ($cs=-1; $cs<10; $cs++) {
  echo "\n----( chunk_size: $cs, output append size: 1 )----\n";
  ObStartBasic::$callback_invocations=0;
  ob_start(fun('callback'), $cs);
  echo '1'; echo '2'; echo '3'; echo '4'; echo '5'; echo '6'; echo '7'; echo '8';
  ob_end_flush();
}

for ($cs=-1; $cs<10; $cs++) {
  echo "\n----( chunk_size: $cs, output append size: 4 )----\n";
  ObStartBasic::$callback_invocations=0;
  ob_start(fun('callback'), $cs);
  echo '1234'; echo '5678';
  ob_end_flush();
}
}
