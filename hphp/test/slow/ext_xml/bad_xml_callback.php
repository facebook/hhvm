<?hh

class foo {
    public static function elementOpen( $parser, $name, $attribs ) {
        print "<$name>";
    }

    public static function elementClose ( $parser, $name ) {
        print "</$name>";
    }
}


<<__EntryPoint>>
function main_bad_xml_callback() {
var_dump( is_callable( varray[ 'foo', 'elementopen', 'elementclose' ] ) );
var_dump( is_callable( 'foo::randomClose' ) );
var_dump( is_callable( varray[ 'foo'] ) );

$parser = xml_parser_create();
xml_set_element_handler(
    $parser,
    varray[ 'foo' ],
    'foo::randomClose'
);

if ( 0 === xml_parse( $parser, "<root><a><b></b></a></root>" ) ) {
    print "Error parsing xml";
}

xml_parser_free( $parser );
}
