<?hh <<__EntryPoint>> function main(): void {
$im = imagecreate(64, 64);
imagecolorallocate($im, 0, 0, 0);

ob_start();
imagegd2($im, '', 64, IMG_GD2_RAW);
$buffer = ob_get_clean();

$header = unpack('@10/nchunk_size/nformat/nx_count/ny_count', $buffer);
printf("chunk size: %d\n", $header['chunk_size']);
printf("x chunk count: %d\n", $header['x_count']);
printf("y chunk count: %d\n", $header['y_count']);
printf("file size: %d\n", strlen($buffer));
}
