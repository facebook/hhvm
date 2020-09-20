<?hh <<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(180, 30);

ob_start();
imagepng($image, '', 9);
$img = ob_get_contents();
ob_end_clean();

echo md5(base64_encode($img));
}
