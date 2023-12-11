<?hh

class aClass {
   function __toString() :mixed{
       return "Some Ascii Data";
   }
}
<<__EntryPoint>>
function entrypoint_xml_parser_get_option_variation2(): void {
  /* Prototype  : proto int xml_parser_get_option(resource parser, int option)
   * Description: Get options from an XML parser
   * Source code: ext/xml/xml.c
   * Alias to functions:
   */

  echo "*** Testing xml_parser_get_option() : usage variations ***\n";
  error_reporting(E_ALL & ~E_NOTICE);
  // Initialise function arguments not being substituted (if any)
  $parser = xml_parser_create();


  $fp = fopen(__FILE__, "r");

  //array of values to iterate over
  $values = vec[

        // outside of range int data
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

        // object data
        new aClass(),

        // resource data
        $fp,


  ];

  // loop through each element of the array for option

  foreach($values as $value) {
        echo @"\nArg value $value \n";
        try { var_dump( xml_parser_get_option($parser, $value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  }

  fclose($fp);
  xml_parser_free($parser);
  echo "Done";
}
