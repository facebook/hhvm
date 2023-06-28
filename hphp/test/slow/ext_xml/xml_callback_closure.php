<?hh

// Stems from https://github.com/facebook/hhvm/issues/4892

function elementOpen( $parser, $name, darray $attribs ) :mixed{
    print "<$name>";
}


<<__EntryPoint>>
function main_xml_callback_closure() :mixed{
var_dump( is_callable( 'elementOpen' ) );
$closure = function ( $parser, $name ) {
            print "</$name>";
          };

var_dump( is_callable( $closure ) );

$parser = xml_parser_create();
xml_set_element_handler(
    $parser,
    'elementOpen',
    function ( $parser, $name ) {
        print "</$name>";
    }
);

if ( 0 === xml_parse( $parser, "<root><a><b></b></a></root>" ) ) {
    print "Error parsing xml";
}

xml_parser_free( $parser );
}
