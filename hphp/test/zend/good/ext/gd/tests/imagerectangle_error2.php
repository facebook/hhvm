<?hh
// Create a resource
<<__EntryPoint>> function main(): void {
$image = tmpfile();
// Draw a rectangle
imagerectangle( $image, 0, 0, 50, 50, 2 );
}
