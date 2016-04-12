<?php

// Stems from https://github.com/facebook/hhvm/issues/4892

class foo {
    public static function elementOpen( $parser, $name, $attribs ) {
        print "<$name>";
    }

    public static function elementClose ( $parser, $name ) {
        print "</$name>";
    }
}

var_dump( is_callable( array( 'foo', 'elementOpen' ) ) );
var_dump( is_callable( 'foo::elementClose' ) );

$parser = xml_parser_create();
xml_set_element_handler(
    $parser,
    array( 'foo', 'elementOpen' ),
    'foo::elementClose'
);

if ( 0 === xml_parse( $parser, "<root><a><b></b></a></root>" ) ) {
    print "Error parsing xml";
}

xml_parser_free( $parser );
