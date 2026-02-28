<?hh
/* Prototype  : proto mixed parse_url(string url, [int url_component])
 * Description: Parse a URL and return its components
 * Source code: ext/standard/url.c
 * Alias to functions:
 */

function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) :mixed{
    echo "Error: $err_no - $err_msg, $filename($linenum)\n";
}
<<__EntryPoint>> function main(): void {
set_error_handler(test_error_handler<>);
echo "*** Testing parse_url() : usage variations ***\n";


//array of values to iterate over
$values = vec[
      // empty data
      "",
      '',
];

// loop through each element of the array for url

foreach($values as $value) {
      echo "\nArg value ".(string)$value."\n";
      try { var_dump( parse_url($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

echo "Done";
}
