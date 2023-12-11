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

// Initialise function arguments not being substituted (if any)
$url = 'http://secret:hideout@www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123';


//array of values to iterate over
$values = vec[

      // float data
      10.5,
      -10.5,
      10.1234567e10,
      10.7654321E-10,
      .5,








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

      // string data
      "string",
      'string',

      // object data
      new stdClass()
];

// loop through each element of the array for url_component

foreach($values as $value) {
      echo "\nArg value ".(string)$value."\n";
      try { var_dump( parse_url($url, $value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

echo "Done";
}
