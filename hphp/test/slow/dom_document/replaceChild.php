<?hh

<<__EntryPoint>>
function main_replace_child() :mixed{
$n = 32;
while ( $n-- ) {
  $doc = new DOMDocument();
  $doc->loadHTML( '<html><body><div id="test"><img /></div></body></html>' );

  $element = $doc->getElementsByTagName( 'img' )->item( 0 );
  $element->parentNode->replaceChild( $doc->createElement( 'img' ), $element );

  $content = $doc->createElement( 'div' );
  $content->appendChild( $doc->getElementById( 'test' ) );
}
echo "Done\n";
}
