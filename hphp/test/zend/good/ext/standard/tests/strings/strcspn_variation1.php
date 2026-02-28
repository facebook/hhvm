<?hh
/* Prototype  : proto int strcspn(string str, string mask [, int start [, int len]])
 * Description: Finds length of initial segment consisting entirely of characters not found in mask.
                If start or/and length is provided works like strcspn(substr($s,$start,$len),$bad_chars)
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
* Testing strspn() : with different unexpected values for str argument
*/

echo "*** Testing strcspn() : with unexpected values for str argument ***\n";

// Initialise function arguments not being substititued (if any)
$mask = 'abons1234567890';
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

// loop through each element of the array for str

foreach($values as $value) {
      $text = HH\is_any_array($value) ? 'Array' : $value; echo "\n-- Iteration with str value as \"$text\"\n";
      try { var_dump( strcspn($value,$mask) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // with default args
      try { var_dump( strcspn($value,$mask,$start) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // with default len value
      try { var_dump( strcspn($value,$mask,$start,$len) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } //  with all args
};

// closing the resource
fclose($file_handle);

echo "Done";
}
