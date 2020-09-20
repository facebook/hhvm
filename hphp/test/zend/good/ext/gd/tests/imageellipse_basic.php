<?hh

// Create a image
<<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(400, 300);
// Draw a white ellipse
imageellipse($image, 200, 150, 300, 200, 16777215);

ob_start();
imagepng($image, '', 9);
$img = ob_get_contents();
ob_end_clean();

echo md5(base64_encode($img));
}
