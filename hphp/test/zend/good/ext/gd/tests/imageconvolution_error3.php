<?hh <<__EntryPoint>> function main(): void {
$image = imagecreatetruecolor(180, 30);

// Writes the text and apply a gaussian blur on the image
imagestring($image, 5, 10, 8, 'Gaussian Blur Text', 0x00ff00);

$gaussian = varray[
    varray[1.0, 2.0, 1.0],
    varray[2.0, 4.0, 2.0],
    varray[1.0, 2.0]
];

var_dump(imageconvolution($image, $gaussian, 16.0, 0.0));
}
