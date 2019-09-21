<?hh <<__EntryPoint>> function main(): void {
$infile = dirname(__FILE__).'/bug72819.jpg';
$width = null;
$height = null;
$type = null;
var_dump(strlen(exif_thumbnail($infile, inout $width, inout $height,
                               inout $type)));
}
