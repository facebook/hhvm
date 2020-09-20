<?hh

// Create a resource
<<__EntryPoint>> function main(): void {
$image = tmpfile();
// try to draw a white ellipse
imageellipse($image, 200, 150, 300, 200, 16777215);
}
