<?hh
// Create a image
<<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor( 100, 100 );
// Draw a rectangle
try { imagerectangle( $image, 0, 0, 50, 50, 'wrong param' ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
