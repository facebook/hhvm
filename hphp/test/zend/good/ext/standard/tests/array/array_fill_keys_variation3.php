<?hh
/* Prototype  : proto array array_fill_keys(array keys, mixed val)
 * Description: Create an array using the elements of the first parameter as keys each initialized to val
 * Source code: ext/standard/array.c
 * Alias to functions:
 */

function _try($fn) :mixed{
  try {
    return $fn();
  } catch (Exception $e) {
    echo get_class($e).': '.$e->getMessage().PHP_EOL;
    return null;
  }
}

/* Testing with unexpected argument types */
<<__EntryPoint>>
function main(): void {
  echo "*** Testing array_fill_keys() : parameter variations ***\n";

  $simpleStr = "simple";
  $fp = fopen(__FILE__, "r");
  $bool = false;
  $float = 2.4;
  $array = vec["one", "two"];
  $nullVal = null;

  echo "\n-- Testing array_fill_keys() function with both wrong arguments --\n";
  var_dump(_try(() ==> array_fill_keys($bool, $float)));

  echo "\n-- Testing array_fill_keys() function with unusual second arguments --\n";
  var_dump(array_fill_keys($array, $fp));

  echo "\n-- Testing array_fill_keys() function with mixed array --\n";
  var_dump(_try(() ==> array_fill_keys($nullVal, $simpleStr)));

  fclose($fp);
  echo "Done";
}
