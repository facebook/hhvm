<?hh
// Create a image
<<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor( 100, 100 );
// Draw a rectangle
imagefilledrectangle( $image, 0, 0, 100, 100, imagecolorallocate( $image, 255, 255, 255 ) );

// Draw an ellipse to fill with a black border
imageellipse( $image, 50, 50, 50, 50, imagecolorallocate( $image, 0, 0, 0 ) );
}
