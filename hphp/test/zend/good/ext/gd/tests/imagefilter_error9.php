<?hh <<__EntryPoint>> function main(): void {
$image = tmpfile();

var_dump(imagefilter($image, IMG_FILTER_GAUSSIAN_BLUR));
}
