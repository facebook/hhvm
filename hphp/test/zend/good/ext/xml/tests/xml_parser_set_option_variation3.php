<?hh

<<__EntryPoint>>
function main_entry(): void {
  /* Prototype  : proto int xml_parser_set_option(resource parser, int option, mixed value)
   * Description: Set options in an XML parser
   * Source code: ext/xml/xml.c
   * Alias to functions:
   */

  echo "*** Testing xml_parser_set_option() : usage variations ***\n";
  error_reporting(E_ALL & ~E_NOTICE);
  // Initialise function arguments not being substituted (if any)

  $parser = xml_parser_create();
  $option = 1;


  $fp = fopen(__FILE__, "r");

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

        // resource data
        $fp,


  ];

  // loop through each element of the array for value

  foreach($values as $value) {
        echo @"\nArg value<$value>\n";
        var_dump( xml_parser_set_option($parser, $option, $value) );
  };

  fclose($fp);
  xml_parser_free($parser);
  echo "Done";
}
