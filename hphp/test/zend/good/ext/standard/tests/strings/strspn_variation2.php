<?hh
/* Prototype  : proto int strspn(string str, string mask [, int start [, int len]])
 * Description: Finds length of initial segment consisting entirely of characters found in mask.
        If start or/and length is provided works like strspn(substr($s,$start,$len),$good_chars)
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

// declaring class
class sample  {
  public function __toString() :mixed{
    return "object";
  }
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL & ~E_NOTICE);

/*
* Testing strspn() : with different unexpected values for mask argument
*/

echo "*** Testing strspn() : with different unexpected values of mask argument ***\n";

$str = 'string_val';
$start = 1;
$len = 10;



// creating a file resource
$file_handle = fopen(__FILE__, 'r');


//array of values to iterate over
$values = vec[
      // empty data
      "",
      '',
];

// loop through each element of the array for mask

foreach($values as $value) {
      $text = HH\is_any_array($value) ? 'Array' : $value; echo "\n-- Iteration with mask value as \"$text\" --\n";
      try { var_dump( strspn($str,$value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // with defalut args
      try { var_dump( strspn($str,$value,$start) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // with default len value
      try { var_dump( strspn($str,$value,$start,$len) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // with all args
};

// close the resource
fclose($file_handle);

echo "Done";
}
