<?hh <<__EntryPoint>> function main(): void {
$image = tmpfile();

var_dump(imagefilter($image, IMG_FILTER_PIXELATE, 3));
}
