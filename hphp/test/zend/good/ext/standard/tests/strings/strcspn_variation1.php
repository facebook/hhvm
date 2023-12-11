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

      // int data
      0,
      1,
      12345,
      -2345,

      // float data
      10.5,
      -10.5,
      10.1234567e10,
      10.7654321E-10,
      .5,

      // array data
      vec[],
      vec[0],
      vec[1],
      vec[1, 2],
      dict['color' => 'red', 'item' => 'pen'],

      // null data
      NULL,
      null,

      // boolean data
      true,
      false,
      TRUE,
      FALSE,

      // empty data
      "",
      '',

      // object data
      new sample,



      // resource
      $file_handle
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
