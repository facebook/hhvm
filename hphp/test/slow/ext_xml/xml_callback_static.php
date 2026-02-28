<?hh

// Stems from https://github.com/facebook/hhvm/issues/4892

class foo {
    public static function elementOpen( $parser, $name, $attribs ) :mixed{
        print "<$name>";
    }

    public static function elementClose ( $parser, $name ) :mixed{
        print "</$name>";
    }
}


<<__EntryPoint>>
function main_xml_callback_static() :mixed{
var_dump( is_callable( vec[ 'foo', 'elementOpen' ] ) );
var_dump( is_callable( 'foo::elementClose' ) );

$parser = xml_parser_create();
xml_set_element_handler(
    $parser,
    vec[ 'foo', 'elementOpen' ],
    'foo::elementClose'
);

if ( 0 === xml_parse( $parser, "<root><a><b></b></a></root>" ) ) {
    print "Error parsing xml";
}

xml_parser_free( $parser );
}
