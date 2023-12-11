<?hh <<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(180, 30);

// Writes the text and apply a gaussian blur on the image
imagestring($image, 5, 10, 8, 'Gaussian Blur Text', 0x00ff00);

$gaussian = vec[
    vec[1.0, 2.0, 1.0],
    vec[2.0, 4.0, 2.0],
    vec[1.0, 2.0, 1.0]
];

try { var_dump(imageconvolution($image, $gaussian, 16)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
