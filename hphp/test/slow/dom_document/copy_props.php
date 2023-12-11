<?hh


<<__EntryPoint>>
function main_copy_props() :mixed{
$template = <<<XML
<?xml version="1.0" encoding="utf-8"?>
<resources></resources>
XML;

$writer = new SimpleXMLElement( $template );

$messages = dict[
  'key' => 'value',
  'anotherkey' => 'anothervalue',
];

foreach ( $messages as $key => $value ) {
  $element = $writer->addChild( 'string', $value );

  $element->addAttribute( 'name', $key );
  // This is non-standard
  if ( $key === 'key' ) {
    $element->addAttribute( 'fuzzy', 'true' );
  }
}

// Make the output pretty with DOMDocument
$dom = new DOMDocument( '1.0' );
$dom->formatOutput = true;
$dom->loadXML( $writer->asXML() );

echo $dom->saveXML();
}
