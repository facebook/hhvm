<?hh <<__EntryPoint>> function main(): void {
$image = new Imagick();
$image->newImage(0, 0, '#dddddd', 'png' );

try {
    $image->roundCorners(5.0, 5.0);
    echo "fail\n";
} catch (ImagickException $e) {
    echo "success\n";
}
}
