<?hh

class aClass {
   function __toString() :mixed{
       return "Some Ascii Data";
   }
}
<<__EntryPoint>>
function entrypoint_xml_parser_set_option_variation2(): void {
  /* Prototype  : proto int xml_parser_set_option(resource parser, int option, mixed value)
   * Description: Set options in an XML parser
   * Source code: ext/xml/xml.c
   * Alias to functions:
   */

  echo "*** Testing xml_parser_set_option() : usage variations ***\n";
  error_reporting(E_ALL & ~E_NOTICE);
  // Initialise function arguments not being substituted (if any)
  $parser = xml_parser_create();


  //array of values to iterate over
  $values = vec[

        // outside of range int data
        12345,
        -2345,

  ];

  // loop through each element of the array for option

  foreach($values as $value) {
        echo @"\nArg value $value \n";
        try { var_dump( xml_parser_set_option($parser, $value, 1) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  }

  xml_parser_free($parser);
  echo "Done";
}
